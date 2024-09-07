import std;
import rapidjson;
import asar;

#define VERSION 1
#define SUBVER 2

using namespace std;
using namespace rapidjson;
using namespace asar;

// =======================================
//   Helper functions
// =======================================

/*
    error(format_string msg, Args... args) -> int: Print error message
    ---
    Input: msg is a format string, args its arguments
    Output: always returns 1. Intended as an exit code
*/
template<class... Args> int error(const format_string<Args...> msg, Args &&... args)
{
    print(cerr, "ERROR: {}",format(msg, args...));
    return 1;
}

/*
    format_path(string* str) -> void: Parse path string
    ---
    Input: str is a reference to a string containing a path
    Output: in-place, now str contains a path completely in lowercase and without quotes
*/
void format_path(string* str)
{
    string path;
    for(auto& c : *str)
        if(c!='\"')
            path.push_back( tolower(c) );
    *str = path;
}

/*
    cleanup(void) -> bool: Erase temporary files
    ---
    Output: true if temporary files were removed, false otherwise
*/
bool cleanup(string tool_folder)
{
    try
    {
        filesystem::remove(tool_folder+"asm/tmp.asm");
        filesystem::remove(tool_folder+"asm/bowsie_defines.asm");
        filesystem::remove(tool_folder+"asm/macro.asm");
        filesystem::remove(tool_folder+"asm/init_ptrs.bin");
        filesystem::remove(tool_folder+"asm/main_ptrs.bin");
        return true;
    }
    catch(filesystem::filesystem_error const & err)
    {
        println("There was an error cleaning up temporary files. Details: {}", err.code().message());
        return false;
    }
}

// =======================================
//   Structures
// =======================================

/*
    struct Settings: BOWSIE settings for de-serialization
    ---
    Check the readme for more info
*/
struct Settings
{
    // Display all info per sprite inserted
    bool verbose;
    // Generate sprite map16
    bool generate_map16;
    // Amount of OW sprites
    int slots;
    // Implementation used to process sprites
    string method;
    // Erase the original game's OW sprite handler
    bool replace_original;
    // Detect the existence of the OMTRE patch
    bool omtre_detect;
    // Directory for custom sprites
    string custom_dir;
    // Ignore RAM boundary
    bool bypass_ram_check;

    /*
        deserialize_json(Document* json, string* err_str) -> bool: Parse settings
        ---
        Input: json is a pointer to the (rapidjson) parsed JSON file
               err_str is a pointer to a string to hold the error data
        Output: true if settings were parsed correctly, false if not.
                err_str now contains any issues found
    */
    bool deserialize_json(Document* json, string* err_str)
    {
        bool status = true;
        static const char * keys[] = {"verbose", "generate_map16", "slots", "method", "replace_original",
                                     "omtre_detect", "custom_dir", "bypass_ram_check"};
        *err_str = "Couldn't find key(s):\t\t\t\t";
        for(const char * key : keys)
        {
            if(!(*json).HasMember(key))
            {
                (*err_str).append(key).append(", ");
                status = false;
            }
        }
        (*err_str) = status ? "" : (*err_str).erase((*err_str).size()-2, 2).append("\n");

        // There's gotta be a better way to do this shit
        if(!(*json)["verbose"].IsBool())
        {
            (*err_str).append("Incorrect data type for verbose:\t\texpected Boolean\n");
            status = false;
        }
        else
            this->verbose = (*json)["verbose"].GetBool();
        if(!(*json)["generate_map16"].IsBool())
        {
            (*err_str).append("Incorrect data type for generate_map16:\t\texpected Boolean\n");
            status = false;
        }
        else
            this->generate_map16 = (*json)["generate_map16"].GetBool();
        if(!(*json)["slots"].IsUint())
        {
            (*err_str).append("Incorrect data type for slots:\t\t\texpected Number (Unsigned Integer)\n");
            status = false;
        }
        else
            this->slots = (*json)["slots"].GetInt();
        if(!(*json)["method"].IsString())
        {
            (*err_str).append("Incorrect data type for method:\t\t\texpected String\n");
            status = false;
        }
        else
        {
            this->method = (*json)["method"].GetString();
            format_path(&(this->method));
        }
        if(!(*json)["replace_original"].IsBool())
        {
            (*err_str).append("Incorrect data type for replace_original:\texpected Boolean\n");
            status = false;
        }
        else
            this->replace_original = (*json)["replace_original"].GetBool();
        if(!(*json)["omtre_detect"].IsBool())
        {
            (*err_str).append("Incorrect data type for omtre_detect:\t\texpected Boolean\n");
            status = false;
        }
        else
            this->omtre_detect = (*json)["omtre_detect"].GetBool();
        if(!( (*json)["custom_dir"].IsNull() || (*json)["custom_dir"].IsString()))
        {
            (*err_str).append("Incorrect data type for custom_dir:\t\texpected Null or String\n");
            status = false;
        }
        else
        {
            this->custom_dir = (*json)["custom_dir"].GetType() ? (*json)["custom_dir"].GetString() : "";
            format_path(&(this->custom_dir));
        }
        if(!(*json)["bypass_ram_check"].IsBool())
        {
            (*err_str).append("Incorrect data type for bypass_ram_check:\texpected Boolean");
            status = false;
        }
        else
            this->bypass_ram_check = (*json)["bypass_ram_check"].GetBool();

        return status;
    }
};

// =======================================

#define HEADER_SIZE 512
#define MAX_SIZE 1024*1024*16
/*
    struct Rom: SMW ROM data
    ---
*/
struct Rom
{
    string rom_path;
    ifstream rom_data;
    int rom_size;
    char * raw_rom_data;
    bool first_time;

    // I/O functions

    /*
        open_rom(void) -> bool: Set ROM file input stream
        ---
        Output: rom_data is an ifstream with the ROM data
                rom_size contains the ROM size in bytes (counting the header!)
                raw_rom_data is a 16MB character array initialized to zeroes
                returns true if the ROM opened successfully, false otherwise
    */
    bool open_rom()
    {
        rom_data = ifstream(rom_path, ios::binary | ios::ate);
        rom_size = rom_data.tellg();
        raw_rom_data = new char[MAX_SIZE] { 0x00 };
        first_time = true;
        return !(!rom_data);
    }

    /*
        done(void) -> void: Close ROM input
        ---
        Output: patched ROM is now written to disk
                raw_rom_data destroyed
    */
    void done()
    {
        ofstream(rom_path, ios::binary).write(raw_rom_data, rom_size);
        delete[] raw_rom_data;
    }

    // ROM data operations

    /*
        read\<int bytes>(int addr) -> uint: Read bytes from ROM
        ---
        Input:  addr is the SNES address to read
                bytes is the amount of bytes to read
        Output: res contains the bytes read (unsigned -1 in failure)
    */
    template<int bytes> unsigned int read(int addr)
    {
        unsigned int res = 0x0;
        rom_data.clear();
        rom_data.seekg(snestopc_pick(addr)+HEADER_SIZE);
        for(int i=0;i<bytes;++i)
            res|=(rom_data.get()&0xFF)<<((bytes-i-1)*8);
        return res;
    }

    /*
        inline_patch(const char * patch_content) -> bool: Patch ROM
        ---
        Input:  patch_content is a char with the patch to apply
        Output: ./asm/tmp.asm contains the patch applied
                returns true if the patch was successfully applied,
                false otherwise
    */
    bool inline_patch(string tool_folder, const char * patch_content)
    {
        int new_size = rom_size - HEADER_SIZE;

        if(first_time)
        {
            rom_data.seekg(512);
            rom_data.read(&(raw_rom_data[HEADER_SIZE]), rom_size);
        }

        ofstream(tool_folder+"asm/tmp.asm").write(patch_content, strlen(patch_content));
        string tmp_path = filesystem::absolute(tool_folder+"asm/tmp.asm").string();
        bool patch_res = asar_patch(tmp_path.c_str(), &(raw_rom_data[HEADER_SIZE]), MAX_SIZE, &new_size);

        if(patch_res)
            first_time = false;
        return patch_res;
    }
};

// =======================================

#define MAP16_SIZE 0x100*8
/*
    struct Map16: OW Sprite map16 generation
    ---
*/
struct Map16
{
    string tooltip;
    int no_tiles;
    vector<int> x_offset;
    vector<int> y_offset;
    int map16_number;

    vector<bool> is_16x16;
    vector<int> tile_num;

    vector<int> y_flip;
    vector<int> x_flip;
    vector<int> priority;
    vector<int> palette;
    vector<int> second_page;

    char * map16_page = new char [MAP16_SIZE] { 0x00 };
    ifstream s16ov;
    ofstream sscov;

    // I/O functions

    /*
        deserialize_json(Document* json, string* err_str) -> bool: Parse Map16 JSON
        ---
        Input: json is a pointer to the (rapidjson) parsed JSON file
               err_str is a pointer to a string to hold the error data
        Output: true if Map16 tooltip information was parsed correctly, false if not.
                err_str now contains any issues found
    */
    bool deserialize_json(Document* json, string* err_str)
    {
        bool status = true;
        static const char * keys[] = {"tooltip", "no_tiles"};
        static const char * tile_keys[] = {"is_16x16", "tile_num", "y_flip", "x_flip", "priority", "palette", "second_page"};
        *err_str = "Couldn't find key(s):\t\t\t\t";
        for(const char * key : keys)
        {
            if(!(*json).HasMember(key))
            {
                (*err_str).append(key).append(", ");
                status = false;
            }
        }
        (*err_str) = status ? "" : (*err_str).erase((*err_str).size()-2, 2).append("\n");

        // There's gotta be a better way to do this shit
        if(!(*json)["tooltip"].IsString())
        {
            (*err_str).append("Incorrect data type for tooltip:\t\texpected String\n");
            status = false;
        }
        else
            this->tooltip = (*json)["tooltip"].GetString();
        if(!(*json)["no_tiles"].IsInt())
        {
            (*err_str).append("Incorrect data type for no_tiles:\t\texpected Integer\n");
            status = false;
        }
        else
            this->no_tiles = (*json)["no_tiles"].GetInt();
        (*err_str) = status ? "Couldn't find tile(s):\t\t\t\t" : (*err_str).erase((*err_str).size()-2, 2).append("\nCouldn't find key(s):\t\t\t\t");

        for(int i=1;i<=no_tiles;++i)
        {
            string c = format("tile_{}", i);
            const char * curr = c.c_str();
            if(!(*json).HasMember(curr))
            {
                (*err_str).append(curr).append(", ");
                status = false;
                break;
            }
            else if(!(*json)[curr].IsObject())
            {
                (*err_str).append(format("\n{} is not a tile (so a JSON structure)\n", curr));
                status = false;
                break;
            }

            (*err_str) = "Couldn't find tile info:\t\t\t\t";

            for(const char * key : tile_keys)
            {
                if(!(*json)[curr].HasMember(key))
                {
                    (*err_str).append(key).append(", ");
                    status = false;
                }
            }
            if(!status)
                break;

            (*err_str) = "";
            if(!(*json)[curr]["is_16x16"].IsBool())
            {
                (*err_str).append("Incorrect data type for is_16x16:\t\texpected Boolean\n");
                status = false;
            }
            else
                this->is_16x16.push_back((*json)[curr]["is_16x16"].GetBool());
            if(!(*json)[curr]["tile_num"].IsInt())
            {
                (*err_str).append("Incorrect data type for tile_num:\t\texpected Integer\n");
                status = false;
            }
            else
                this->tile_num.push_back((*json)[curr]["tile_num"].GetInt());
            if(!(*json)[curr]["x_offset"].IsInt())
            {
                (*err_str).append("Incorrect data type for x_offset:\t\texpected Integer\n");
                status = false;
            }
            else
                this->x_offset.push_back((*json)[curr]["x_offset"].GetInt());
            if(!(*json)[curr]["y_offset"].IsInt())
            {
                (*err_str).append("Incorrect data type for y_offset:\t\texpected Integer\n");
                status = false;
            }
            else
                this->y_offset.push_back((*json)[curr]["y_offset"].GetInt());
            if(!(*json)[curr]["y_flip"].IsBool())
            {
                (*err_str).append("Incorrect data type for y_flip:\t\texpected Boolean\n");
                status = false;
            }
            else
                this->y_flip.push_back((*json)[curr]["y_flip"].GetInt());
            if(!(*json)[curr]["x_flip"].IsBool())
            {
                (*err_str).append("Incorrect data type for x_flip:\t\texpected Boolean\n");
                status = false;
            }
            else
                this->x_flip.push_back((*json)[curr]["x_flip"].GetBool());
            if(!(*json)[curr]["priority"].IsInt())
            {
                (*err_str).append("Incorrect data type for priority:\t\texpected Integer\n");
                status = false;
            }
            else
                this->priority.push_back((*json)[curr]["priority"].GetInt());
            if(!(*json)[curr]["palette"].IsInt())
            {
                (*err_str).append("Incorrect data type for palette:\t\texpected Integer\n");
                status = false;
            }
            else
                this->palette.push_back((*json)[curr]["palette"].GetInt());
            if(!(*json)[curr]["second_page"].IsBool())
            {
                (*err_str).append("Incorrect data type for second_page:\t\texpected Boolean\n");
                status = false;
            }
            else
                this->second_page.push_back((*json)[curr]["second_page"].GetBool());
        }

        return status;
    }

    /*
        open_s16ov(const char * filename) -> void: open .s16ov
        ---
        Input:  filename is the ROM filepath+name (no extension)
        Output: s16ov is an ifstream with the map16 data
    */
    void open_s16ov(const char * filename)
    {
        ofstream(filename, ios::binary).write(map16_page, MAP16_SIZE);
        s16ov = ifstream(filename, ios::binary);
    }

    /*
        open_sscov(const char * filename) -> void: open .sscov
        ---
        Input:  filename is the ROM filepath+name (no extension)
        Output: s16ov is an ifstream with the custom sprite map16 mappings data
    */
    void open_sscov(const char * filename)
    {
        ofstream(filename, ios::app).write("", 1);
        sscov = ofstream(filename, ios::app);
    }

    /*
        done(const char * filename) -> void: write .s16ov file to disk
        ---
        Input:  filename is the ROM filepath+name (no extension)
    */
    void done(const char * filename)
    {
        ofstream(filename, ios::binary).write(map16_page, MAP16_SIZE);
        delete[] map16_page;
    }

    // Sprite map16 tilemap functions

    /*
        get_map16_tile(int step) -> int: get next free tile in .s16ov file
        ---
        Input:  step is the amount to jump (normally 2 for one 8x8 tile, 8 for one 16x16 tile)
        Output: i is the offset where to put the tile. -1 if none
    */
    int get_map16_tile(int step)
    {
        for(int i=0;i<MAP16_SIZE;i+=step)
        {
            if((map16_page[i]|map16_page[i+1]|map16_page[i+2]|map16_page[i+3]|map16_page[i+4]|map16_page[i+5]|map16_page[i+6]|map16_page[i+7])==0x00)
                return i;
        }
        return -1;
    }

    /*
        write_single_map16_tile(int tile_number, int yxppccct, int pos) -> bool: Write map16 tile info to memory
        ---
        Input:  tile_number is the GFX position of the tile
                yxppccct is the tile info (Y flip, X flip, priority, palette, page)
                pos is where to write
        Output: true in success, false otherwise
                map16_page now contains the tile info
    */
    bool write_single_map16_tile(int tile_number, int yxppccct, int pos)
    {
        if(pos == -1 || pos >= MAP16_SIZE)
            return false;
        map16_page[pos] = tile_number;
        map16_page[pos+1] = yxppccct;
        return true;
    }

    /*
        write_map16_tiles(string * err_string) -> int *: Write set of map16 tiles
        ---
        Input:  deserialize_json has been run
                *err_string is a pointer to a string to hold error data
        Output: the vectors with information must be empty
                returns an integer array with the tile position in the page. if it
                contains -1, then an error occurred. err_string contains said info
    */
    int * write_map16_tiles(string * err_string)
    {
        int * map16_numbers = new int [no_tiles] { 0x00 };
        for(int i=0; i<no_tiles; ++i)
        {
            int pos = get_map16_tile(is_16x16[i] ? 8 : 2);
            if(pos == -1 || pos > MAP16_SIZE-8)
            {
                map16_numbers[i] = -1;
                (*err_string).append(format("Not enough map16 space while starting to write a tile. Position is {}", pos));
                return map16_numbers;
            }
            int tile = tile_num[i];
            int yxppccct = (y_flip[i]<<7)|(x_flip[i]<<6)|((priority[i]&3)<<4)|((palette[i]%8)<<2)|(second_page[i]);
            tile_num.pop_back();
            y_flip.pop_back();
            x_flip.pop_back();
            priority.pop_back();
            palette.pop_back();
            second_page.pop_back();
            if(is_16x16[i])
            {
                if(!write_single_map16_tile(tile, yxppccct, pos) ||
                   !write_single_map16_tile(tile+0x10, yxppccct, pos+2) ||
                   !write_single_map16_tile(tile+0x01, yxppccct, pos+4) ||
                   !write_single_map16_tile(tile+0x11, yxppccct, pos+6))
                {
                    map16_numbers[i] = -1;
                    (*err_string).append(format("Not enough map16 space while writing part of a 16x16 tile. Position is {}", pos));
                    return map16_numbers;
                }
                map16_numbers[i] = (pos/8)+0x400;
            }
            else
            {
                if(!write_single_map16_tile(tile, yxppccct, pos))
                {
                    map16_numbers[i] = -1;
                    (*err_string).append(format("Not enough map16 space while writing an 8x8 tile. Position is {}", pos));
                    return map16_numbers;
                }
                map16_numbers[i] = (pos/8)+0x400;
            }
        }
        is_16x16.assign({});
        return map16_numbers;
    }

    // Tooltip (and main) function

    /*
        write_tooltip(int sprite_number, string * err_string) -> bool: Write tooltip for custom sprite
        ---
        Input:  sprite_number is the OW sprite slot
                *err_string is a pointer to a string to hold error data
        Output: true in success, false otherwise.
                if false, err_string now contains the error info
    */
    bool write_tooltip(int sprite_number, string * err_string)
    {
        string str = format("{:x}\t10\t{}\n", sprite_number, tooltip);
        sscov.write(str.c_str(), str.size());
        int * map16_numbers = write_map16_tiles(err_string);
        for(int i=0;i<no_tiles;i+=4)
        {
            if(map16_numbers[i]==-1)
                return false;
            str = format("{:x}\t12\t{},{},{:X}\n", sprite_number, x_offset[i], y_offset[i], map16_numbers[i]);
            sscov.write(str.c_str(), str.size());
            x_offset.pop_back();
            y_offset.pop_back();
        }
        x_offset.assign({});
        y_offset.assign({});
        return true;
    }

};

/*
    destroy_map16(string filename) -> void: delete OW sprite map16 files
    ---
    Input:  filename is the ROM filepath+name (no extension)
    Output: sprite map16 files erased from disk (if they existed)
*/
void destroy_map16(string filename)
{
    if(filesystem::exists(filename+".s16ov"))
        filesystem::remove(filename+".s16ov");
    if(filesystem::exists(filename+".sscov"))
        filesystem::remove(filename+".sscov");
}

// =======================================
//   Main function!!!
// =======================================

#define BOWSIE_USED_PTR 0x04EF3E  // must be freespace in bank 4
#define MAGIC_CONSTANT 0x00CAC705 // 00cactus
#define MAGIC_CONSTANT_WRITE ((MAGIC_CONSTANT&0xFF)<<24)|((MAGIC_CONSTANT&0xFF00)<<8)|((MAGIC_CONSTANT&0xFF0000)>>8)|((MAGIC_CONSTANT&0xFF000000)>>24)

int main(int argc, char *argv[])
{
    println("BOWSIE - Better Overworld Sprite Insertion Engine");
    println("\t v{}.{:0>2}", VERSION, SUBVER);
    println("\t By Erik\n");

    Rom rom;
    string list_path;
    Settings settings;

    /*
        Parse inputs
    */
    switch(argc)
    {
        case 0:
        case 1:
            while(1)
            {
                print("Select ROM file: ");
                getline(cin, rom.rom_path);
                format_path(&rom.rom_path);
                if(!rom.rom_path.ends_with(".smc") && !rom.rom_path.ends_with(".sfc"))
                    error("Not a valid ROM file: {}\n", rom.rom_path);
                else
                    break;
            }
            while(1)
            {
                print("Select list file (.txt only): ");
                getline(cin, list_path);
                format_path(&list_path);
                if(!list_path.ends_with(".txt"))
                    error("Not a valid list file: {}\n", list_path);
                else
                    break;
            }
            break;
        case 2:
        {
            string s (argv[1]);
            format_path(&s);
            if(s=="-h" || s=="--help")
            {
                println("\t bowsie [switch] [rom] [list]\n");
                println("Switch");
                println("  -h, --help\t\tDisplay this message and quit\n");
                println("Configuration file settings (bowsie_setings.json)");
                println("  verbose\t\tDisplay all info per sprite inserted");
                println("  generate_map16\tCreate .s16ov and .sscov files for LM display");
                println("  slots\t\t\tAmount of OW sprites (max. 24)");
                println("  method\t\tImplementation used to process sprites");
                println("\t\t\t  - vldc9:   the system used in VLDC9");
                println("\t\t\t  - katrina: Katrina's alternative RAM definitions for VLDC9");
                println("\t\t\t  - owrev:   replace the OW Revolution system");
                println("\t\t\t  - custom:  any user-specified handler");
                println("  replace_original\tErase the original game's OW sprite handler/sprites");
                println("  omtre_detect\t\tIf replace_original is false, detect the existence");
                println("\t\t\t  of the Overworld Mario Tilemap Rewrite and Expand (OMTRE)");
                println("\t\t\t  patch and insert the code in there\n");
                println("Advanced settings (don't touch if you don't know what you're doing)");
                println("  custom_dir\t\tPath to the asm file which handles the new OW sprites");
                println("  bypass_ram_check\tIgnore RAM boundaries (and by extension, sprite limit)");
            }
            else
                return error("Unknown command: {}", s);
        }
            return 0;
        case 3:
        {
            string path1 (argv[1]);
            string path2 (argv[2]);
            format_path(&path1);
            format_path(&path2);
            if( (path1.ends_with(".smc") || path1.ends_with(".sfc")) && path2.ends_with(".txt") )
            {
                rom.rom_path = path1;
                list_path = path2;
            }
            else if( (path2.ends_with(".smc") || path2.ends_with(".sfc")) && path1.ends_with(".txt") )
            {
                rom.rom_path = path2;
                list_path = path1;
            }
            else
                return error("Expected both a ROM file and a list file.");
            break;
        }
        default:
            return error("Too many arguments.");
    }

    /*
        Existence checks
    */
    if(!rom.open_rom())
        return error("Couldn't open ROM file {}", rom.rom_path);
    ifstream list(list_path);
    if(!list)
        return error("Couldn't open list file {}", list_path);

    /*
        Parse settings
    */
    Document settings_json;
    // First we look for bowsie-config.json in the ROM path
    // if not there, we look in the root folder
    {
        string tmp(filesystem::relative(rom.rom_path).string());
        #ifdef _WIN32
            string settings_path(tmp.begin(), tmp.begin()+tmp.find_last_of("\\")+1);
        #else
            string settings_path(tmp.begin(), tmp.begin()+tmp.find_last_of("/")+1);
        #endif
        settings_path.append("bowsie-config.json");

        ifstream ifs(settings_path);
        if(!ifs)
        {
            tmp = argv[0];
            settings_path = tmp.append("/../bowsie-config.json");

            ifs = ifstream(settings_path);
            if(!ifs)
                return error("Couldn't find the settings file. Please ensure bowsie-settings.json is either in the ROM or the tool directory.");
        }
        BasicIStreamWrapper isw(ifs);

        
        settings_json.ParseStream(isw);
        if(settings_json.HasParseError())
            return error("A problem occurred while parsing bowsie-settings.json. Please ensure the file isn't corrupted and has a valid JSON format. Check the readme for more information.");
    }
    string settings_err;
    if(!settings.deserialize_json(&settings_json, &settings_err))
        return error("A problem occurred while parsing specific settings. Details:\n{}", settings_err);

    /*
        Verify settings
    */
    if(settings.slots==0)
        return error("Can't have zero slots. It makes no sense.");
    else if(settings.slots>24 && !(settings.bypass_ram_check))
        return error("Can't have more than 24 slots due to memory limitations.");
    else if( settings.bypass_ram_check && settings.method!="custom" && settings.slots>255 )
        return error("Can't have more than 255 slots without a custom insertion handler.");

    if(settings.method!="vldc9" && settings.method!="katrina" &&  settings.method!="owrev" && settings.method!="custom")
        return error("Unknown insertion method: {}", settings.method);

    if(settings.verbose)
    {
        println("\nVerbose mode enabled.");
        println("Running BOWSIE with");
        println("Slots:\t\t\t\t{}", settings.slots);
        println("Insertion method:\t\t{}", settings.method);
        println("Replacing original sprites:\t{}", settings.replace_original ? "Yes" : "No");
        println("OMTRE check:\t\t\t{}", settings.omtre_detect ? "Enabled" : "Disabled");
        println("RAM checks:\t\t\t{}", settings.bypass_ram_check ? "Ignored (!)" : "Enabled");
        println("Asar version: v{}.{}{}\n", (int)(((asar_version()%100000)-(asar_version()%1000))/10000),
                                            (int)(((asar_version()%1000)-(asar_version()%10))/100),
                                            (int)(asar_version()%10));
    }
    else
        println("");

    int asar_errcount = 0;
    string tool_folder = filesystem::absolute(argv[0]).parent_path().string()+"/";
    string rom_name = filesystem::absolute(rom.rom_path).parent_path().string()+"/"+filesystem::path(rom.rom_path).stem().string();

    // OW Revolution check
    bool ow_rev = false;
    if(rom.read<2>(0x048000)==0x5946 && rom.read<2>(0x048002)==0x4F52)
    {
        ow_rev = true;
        if(settings.verbose)
            println("Overworld Revolution detected.\nWARNING: Support is experimental!\n");
    }

    // Misc ROM checks
    // The OW save check is just customary. I'm sure it isn't needed, but hey, better safe than sorry.
    if(rom.rom_size<0x100000)
        return error("This ROM is clean. Please edit it in Lunar Magic.");
    if((rom.read<3>(0x00FFC0))!=0x535550) // SUP
        return error("Title mismatch. Is this ROM headered?");
    if(rom.read<1>(0x049549)!=0x22 && !ow_rev)
        return error("Please save the overworld at least once in Lunar Magic.");

    string full_patch(format("incsrc \"asm/{}_defines.asm\"\nincsrc \"asm/macro.asm\"\n\n", settings.method));
    {
        string tmp;
        // Insert the method
        {
            ifstream method_patch;
            if(settings.method=="custom")
            {
                method_patch.open(tool_folder+"asm/"+settings.custom_dir, ios::ate);
                if(!method_patch)
                    return error("Couldn't open custom method {}. Make sure it's in the asm folder.", settings.custom_dir);
            }
            else
            {
                method_patch.open(tool_folder+"asm/"+settings.method+".asm", ios::ate);
                if(!method_patch)
                    return error("Couldn't open {}.asm. Make sure it's in the asm folder.", settings.method);
            }
            method_patch.seekg(0);
            getline(method_patch, tmp, '\0');
        }
        full_patch.append(tmp);
    }

    // Figure out whether the tool's been used and clean-up if so
    if( (rom.read<2>(BOWSIE_USED_PTR)==(MAGIC_CONSTANT&0xFFFF0000)>>16) && (rom.read<2>(BOWSIE_USED_PTR+2)==(MAGIC_CONSTANT&0xFFFF)) )
    {
        if(settings.verbose)
            println("Performing clean-up of a previous execution...");
        destroy_map16(rom_name);

        string clean_patch;
        int clean_offset = 4;
        while(1)
        {
            if(rom.read<3>(BOWSIE_USED_PTR+clean_offset)==0x555555)
            {
                clean_patch.append(format("org ${:0>6X}\n    dl $FFFFFF", BOWSIE_USED_PTR+clean_offset));
                break;
            }
            clean_patch.append(format("autoclean read3(${0:0>6X})\norg ${0:0>6X}\n    dl $FFFFFF\n", BOWSIE_USED_PTR+clean_offset));
            clean_offset+=3;
        }
        if(!rom.inline_patch(tool_folder, clean_patch.c_str()))
            return error("An error ocurred while cleaning up. Details:\n  {}", asar_geterrors(&asar_errcount)->fullerrdata);
        else if(settings.verbose)
            println("Clean-up done.\n");
    }
    else
    {
        println("First time using BOWSIE! Thank you. :)\n");
        rom.inline_patch(tool_folder, format("org ${:0>6X} : dd ${:0>8X}", BOWSIE_USED_PTR, MAGIC_CONSTANT_WRITE).c_str());
    }

    // Check if custom OW sprites have been enabled.
    if(rom.read<3>(0x0EF55D)==0xFFFFFF && !ow_rev)
        return error("Custom OW sprites haven't been enabled in Lunar Magic. Enable them (Ctrl-Insert while in sprite edit mode) and run the tool again.");

    // BOWSIE-specific defines
    string defines(format("\
!bowsie_ow_slots = {}\n\
!bowsie_replace_original = {}\n\
!bowsie_omtre = {}\n\
!bowsie_version = {}\n\
!bowsie_subversion = {}\n\n\
!map_offsets = read3($0EF55D)\n\n\
!sa1 = 0\n\
!dp = $0000\n\
!addr = $0000\n\
!bank = $800000\n\
if read1($00FFD5) == $23\n\
    sa1rom\n\
    !sa1 = 1\n\
    !dp = $3000\n\
    !addr = $6000\n\
    !bank = $000000\n\
endif\n\n\
!bowsie_owrev = 0\n\
if read4($048000) == $524F4659\n\
    !bowsie_owrev = 1\n\
endif\n", settings.slots, settings.replace_original ? '1' : '0', settings.omtre_detect ? '1' : '0', VERSION, SUBVER));
    /*
        these loops locate where certain code is inserted by overworld revolution.
        since the patch has many variables, these hijack spots are NEVER fixed.
    */
    const int owrev_draw_player_routine = (rom.read<1>(0x04811C)<<16)|(rom.read<1>(0x04811B)<<8)|(rom.read<1>(0x04811A));
    if(ow_rev)
    {
        // Freespace
        for(int i=0;;++i)
        {
            if(rom.read<4>(0x04ACD0+i)==0x00000000)
            {
                defines.append(format("!owrev_bank_4_freespace = ${:0>6X}\n", 0x04ACD0+i));
                if(settings.verbose)
                    println("Using OW Revolution bank 4 freespace located at ${:0>6X}\n", 0x04ACD0+i);
                break;
            }
        }
        defines.append(format("!owrev_draw_player_routine #= ${:0>6X}", owrev_draw_player_routine));
    }
    ofstream(tool_folder+"asm/bowsie_defines.asm").write(defines.c_str(), defines.size());

    // Shared sub-routines
    string routine_macro(format("incsrc {}_defines.asm\n\n", settings.method));
    string routine_content("freecode cleaned\n\n");
    vector<string> routine_names;
    for(auto routine : filesystem::directory_iterator(tool_folder+"routines"))
    {
        string routine_path(routine.path().string());
        string routine_name(routine.path().stem().string());
        routine_names.push_back(routine_name);
        routine_macro.append(format("macro {0}()\n    JSL {0}_{0}\nendmacro\n\n", routine_name));
        routine_content.append(format("namespace {0}\n    {0}:\n        incsrc \"{1}\"\n    print \"Shared subroutine %{0}() inserted at $\", hex({0})\nnamespace off\n\n",\
        routine_name, routine_path));
    }

    int offset = 0;
    if(!rom.inline_patch(tool_folder, (routine_macro+routine_content).c_str()))
        return error("An error ocurred while inserting shared subroutines. Details:\n  {}", asar_geterrors(&asar_errcount)->fullerrdata);
    else
    {
        routine_macro = "";
        routine_content = "";
        if(settings.verbose)
            println("===========================================================");
        auto routine_print = asar_getprints(&asar_errcount);
        for(int i=0;i<asar_errcount;++i)
        {
            string routine_addr(routine_print[i]);
            routine_addr = string(routine_addr.begin()+routine_addr.find_first_of("$"), routine_addr.end());
            routine_macro.append(format("macro {}()\n    JSL {}\nendmacro\n\n", routine_names[i], routine_addr));
            routine_content.append(format("org ${:0>6X}\n    dl {}\n", BOWSIE_USED_PTR+4+(i*3), routine_addr));
            if(settings.verbose)
            {
                println("{}", routine_print[i]);
                println("-----------------------------------------------------------");
            }
        }
        println("{} shared subroutines inserted.\n{}\n", asar_errcount, settings.verbose ? "===========================================================" : "");
        offset = asar_errcount;
    }
    ofstream(tool_folder+"asm/macro.asm").write(routine_macro.c_str(), routine_macro.size());
    rom.inline_patch(tool_folder, routine_content.c_str());

    // Pointers
    unsigned char * ow_init_ptrs= new unsigned char[0x7E*3] {};
    unsigned char * ow_main_ptrs= new unsigned char[0x7E*3] {};
    for(int i=0;i<0x7E;++i)
    {
        if(!ow_rev)
        {
            ow_init_ptrs[i*3] = 0x14;
            ow_main_ptrs[i*3] = 0x14;
            ow_init_ptrs[1+i*3] = 0x84;
            ow_main_ptrs[1+i*3] = 0x84;
            ow_init_ptrs[2+i*3] = 0x04;
            ow_main_ptrs[2+i*3] = 0x04;
        }
        else
        {
            ow_init_ptrs[i*3] = 0xFF;
            ow_main_ptrs[i*3] = 0xFF;
            ow_init_ptrs[1+i*3] = 0xFF;
            ow_main_ptrs[1+i*3] = 0xFF;
            ow_init_ptrs[2+i*3] = 0x04;
            ow_main_ptrs[2+i*3] = 0x04;
        }
    }

    // Sprites
    int sprite_count = 0;
    Map16 map16;
    if(settings.generate_map16)
    {
        map16.open_s16ov(string(rom_name+".s16ov").c_str());
        map16.open_sscov(string(rom_name+".sscov").c_str());
    }
    println("===========================================================");
    for(string sprite; getline(list, sprite);)
    {
        try
        {
            size_t pos {};
            int sprite_number = stoi(sprite, &pos, 16);
            if(sprite_number<0x01 || sprite_number>0x7F) throw out_of_range("");
            string sprite_filename(sprite.substr(sprite.find_first_not_of("\t ", pos)));
            format_path(&sprite_filename);
            if(!sprite_filename.ends_with(".asm") && !sprite_filename.ends_with(".asm\""))
                return error("Unknown extension for sprite {}. (Remember the list file looks for the .asm file, NOT the .json tooltip!)", sprite_filename);
            string sprite_labelname(sprite_filename.substr(0, sprite_filename.find_first_of("."))+"_"+to_string(sprite_number));

            if(!ow_rev && (ow_init_ptrs[2+(sprite_number-1)*3]<<16 | ow_init_ptrs[1+(sprite_number-1)*3]<<8 | ow_init_ptrs[(sprite_number-1)*3])!=0x048414 ) throw invalid_argument("");
            if(ow_rev && (ow_init_ptrs[2+(sprite_number-1)*3]<<16 | ow_init_ptrs[1+(sprite_number-1)*3]<<8 | ow_init_ptrs[(sprite_number-1)*3])!=0x04FFFF ) throw invalid_argument("");

            ifstream curr_sprite(tool_folder+"sprites/"+sprite_filename);
            if(!curr_sprite)
                return error("Could not open sprite with number {:0>2X} and filename {}. Make sure the sprite exists and is in the sprites directory.", sprite_number, sprite_filename);
            string insert_sprites(format("incsrc {0}_defines.asm\n\
incsrc macro.asm\n\n\
namespace {1}\n\
    freecode cleaned\n\
    incsrc \"sprites/{2}\"\n\
    print \"Sprite {3:0>2X} - {2}\"\n\
    print \"    Init routine inserted at: $\", hex({1}_init)\n\
    print \"    Main routine inserted at: $\", hex({1}_main)\n\
namespace off\n", settings.method, sprite_labelname, sprite_filename, sprite_number));
            if(!rom.inline_patch(tool_folder, insert_sprites.c_str()))
                return error("Could not insert sprite {}. Details: {}\n", sprite_filename, asar_geterrors(&asar_errcount)->fullerrdata);
            else
            {
                bool init = true;
                auto sprite_print = asar_getprints(&asar_errcount);
                size_t pos {};
                for(int i=0;i<asar_errcount;++i)
                {
                        string sprite_addr(sprite_print[i]);
                        println("{}", sprite_addr);
                        if(sprite_addr.contains('$'))
                        {
                            sprite_addr = string(sprite_addr.begin()+sprite_addr.find_first_of("$")+1, sprite_addr.end());
                            if(init)
                            {
                                init = false;
                                ow_init_ptrs[2+(sprite_number-1)*3] = stoi(string(sprite_addr.begin(),sprite_addr.begin()+2), &pos, 16);
                                ow_init_ptrs[1+(sprite_number-1)*3] = stoi(string(sprite_addr.begin()+2,sprite_addr.begin()+4), &pos, 16);
                                ow_init_ptrs[0+(sprite_number-1)*3] = stoi(string(sprite_addr.begin()+4,sprite_addr.begin()+6), &pos, 16);
                            }
                            else
                            {
                                init = true;
                                ow_main_ptrs[2+(sprite_number-1)*3] = stoi(string(sprite_addr.begin(),sprite_addr.begin()+2), &pos, 16);
                                ow_main_ptrs[1+(sprite_number-1)*3] = stoi(string(sprite_addr.begin()+2,sprite_addr.begin()+4), &pos, 16);
                                ow_main_ptrs[0+(sprite_number-1)*3] = stoi(string(sprite_addr.begin()+4,sprite_addr.begin()+6), &pos, 16);
                            }
                        }
                }
            }
            if(settings.generate_map16)
            {
                string sprite_tooltip_path = tool_folder+"sprites/"+string(sprite_filename.begin(), sprite_filename.begin()+sprite_filename.find_first_of(".")+1)+"json";
                ifstream ifs(sprite_tooltip_path);
                if(!(!ifs))
                {
                    Document sprite_tooltip;
                    BasicIStreamWrapper isw(ifs);
                    string e;
                    if(settings.verbose)
                        println("Parsing tooltip information for {}...", sprite_filename);
                    sprite_tooltip.ParseStream(isw);
                    if(sprite_tooltip.HasParseError())
                        return error("A problem occurred while parsing {}. Please ensure the file isn't corrupted and has a valid JSON format.", sprite_tooltip_path);
                    if(!map16.deserialize_json(&sprite_tooltip, &e))
                        return error("A problem occurred while parsing specific tooltips for {}. Details:\n{}", sprite_tooltip_path, e);
                    e = "";
                    if(!map16.write_tooltip(sprite_number, &e))
                        return error("A problem has ocurred while generating tooltips for {}. Details:\n{}", sprite_tooltip_path, e);
                    else if(settings.verbose)
                        println("Done.");
                }
                else if(settings.verbose)
                    println("{} has no tooltip information.", sprite_filename);
            }
            println("-----------------------------------------------------------");
            ++sprite_count;
        }
        catch(invalid_argument const & err)
        {
            return error("Error parsing the list file: duplicate sprite number\n  {}\n", sprite);
        }
        catch(out_of_range const & err)
        {
            return error("Error parsing the list file: incorrect sprite number\n  {}\n", sprite);
        }
    }
    println("{} sprites inserted.\n===========================================================", sprite_count);
    ofstream(tool_folder+"asm/init_ptrs.bin", ios::binary).write((char *)ow_init_ptrs, 0x7E*3);
    ofstream(tool_folder+"asm/main_ptrs.bin", ios::binary).write((char *)ow_main_ptrs, 0x7E*3);

    // Sprite system
    full_patch.append(format("\norg ${:0>6X}\n\
    ow_sprite_init_ptrs:\n\
        incbin init_ptrs.bin\n\
    ow_sprite_main_ptrs:\n\
        incbin main_ptrs.bin\n\n\
    dl $555555", BOWSIE_USED_PTR+4+(offset*3)));
    if(ow_rev)
        full_patch.append("\npushpc\norg $04FFFF\n    RTL\npullpc");
    if(!rom.inline_patch(tool_folder, full_patch.c_str()))
        return error("Something went wrong while applying the sprite system. Details:\n  {}", asar_geterrors(&asar_errcount)->fullerrdata);

    // Done
    println("All sprites inserted successfully!\nRemember to run the tool again when you insert a custom OW sprite in Lunar Magic.");
    map16.done(string(rom_name+".s16ov").c_str());
    rom.done();
    if(!cleanup(tool_folder))
        return error("Error cleaning up temporary files. Exiting... (Your ROM was still saved anyway)");
    return 0;
}

// cl /nologo /Febowsie /std:c++latest /EHsc /O2 /W2 /reference "std=std.ifc" /reference "rapidjson=rapidjson.ifc" /reference "asar=asar.ifc" bowsie.cpp std.obj rapidjson.obj asar.obj /link bowsie.res
// rc /nologo bowsie.rc

// for_each(rom_path.begin()+rom_path.size()-3,rom_path.end(),[](char &c){c = toupper(c);});

// #define LOWERCASE(str) for_each(str.begin(),str.end(),[](char & c) \
// { c = tolower(static_cast<unsigned char>(c));})