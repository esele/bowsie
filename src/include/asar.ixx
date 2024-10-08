module;

#include <Windows.h>
#include <ctype.h>

export module asar;

#pragma warning(push)
#pragma warning(disable: 5244 4146 4244)

#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define ASAR_STATIC 1

#include "asar/asar.h"

#include "asar/addr2line.h"
#include "asar/arch-shared.h"
#include "asar/asar_math.h"
#include "asar/assembleblock.h"
#include "asar/assocarr.h"
#include "asar/autoarray.h"
#include "asar/crc32.h"
#include "asar/errors.h"
#include "asar/interface-shared.h"
#include "asar/libcon.h"
#include "asar/libmisc.h"   // no joke msvc was crying because the inline function was called min
#include "asar/libsmw.h"
#include "asar/libstr.h"
#include "asar/macro.h"
#include "asar/std-includes.h"
#include "asar/virtualfile.h"
#include "asar/warnings.h"

#include "asar/platform/file-helpers.h"

#include "asar/main.cpp"
#include "asar/addr2line.cpp"
#include "asar/arch-65816.cpp"
#include "asar/arch-spc700.cpp"
#include "asar/arch-superfx.cpp"
#include "asar/asar_math.cpp"
#include "asar/assembleblock.cpp"
#include "asar/crc32.cpp"
#include "asar/errors.cpp"
#include "asar/libcon.cpp"
#include "asar/libsmw.cpp"
#include "asar/libstr.cpp"
#include "asar/macro.cpp"
#include "asar/virtualfile.cpp"
#include "asar/warnings.cpp"

#include "asar/platform/file-helpers.h"

#include "asar/platform/file-helpers.cpp"
#include "asar/platform/windows/file-helpers-win32.cpp"

static autoarray<const char *> prints;
static string symbolsfile;
static int numprint;
static uint32_t romCrc;

struct patchparams_base {
    int structsize;
};

struct errordata {
    const char * fullerrdata;
    const char * rawerrdata;
    const char * block;
    const char * filename;
    int line;
    const char * callerfilename;
    int callerline;
    int errid;
};
static  autoarray<errordata> errors;
static int numerror;

static autoarray<errordata> warnings;
static int numwarn;

struct labeldata {
    const char * name;
    int location;
};

struct definedata {
    const char * name;
    const char * contents;
};

struct warnsetting {
    const char * warnid;
    bool enabled;
};

struct memoryfile {
    const char* path;
    const void* buffer;
    size_t length;
};

void print(const char * str)
{
    prints[numprint++]= duplicate_string(str);
}

static void fillerror(errordata& myerr, int errid, const char * type, const char * str, bool show_block)
{
    myerr.filename= duplicate_string(thisfilename);
    myerr.line=thisline;
    if (thisblock) myerr.block= duplicate_string(thisblock);
    else myerr.block= duplicate_string("");
    myerr.rawerrdata= duplicate_string(str);
    myerr.fullerrdata= duplicate_string(STR getdecor()+type+str+((thisblock&&show_block)?(STR" ["+thisblock+"]"):STR ""));
    myerr.callerline=callerline;
    myerr.callerfilename=callerfilename ? duplicate_string(callerfilename) : nullptr;
    // RPG Hacker: TODO: Rework this once we bump the DLL API version again.
    myerr.errid = errid;
}

static bool ismath=false;
static string matherror;

void error_interface(int errid, int whichpass, const char * e_)
{
    errored = true;
    if (ismath) matherror = e_;
    else if (pass == whichpass) {
        // don't show current block if the error came from an error command
        bool show_block = (errid != error_id_error_command);
        fillerror(errors[numerror++], errid, STR "error: (" + get_error_name((asar_error_id)errid) + "): ", e_, show_block);
    }
    else {}//ignore anything else
}

void warn(int errid, const char * str)
{
    // don't show current block if the warning came from a warn command
    bool show_block = (errid != warning_id_warn_command);
    fillerror(warnings[numwarn++], errid, STR "warning: (" + get_warning_name((asar_warning_id)errid) + "): ", str, show_block);
}

static autoarray<labeldata> ldata;
static int labelsinldata = 0;
static autoarray<definedata> ddata;
static int definesinddata=0;

static void resetdllstuff()
{
#define free_and_null(x) free((void*)x); x = nullptr
    for (int i=0;i<numprint;i++)
    {
        free_and_null(prints[i]);
    }
    prints.reset();
    numprint=0;

    for (int i=0;i<numerror;i++)
    {
        free_and_null(errors[i].filename);
        free_and_null(errors[i].rawerrdata);
        free_and_null(errors[i].fullerrdata);
        free_and_null(errors[i].callerfilename);
        free_and_null(errors[i].block);
    }
    errors.reset();
    numerror=0;

    for (int i=0;i<numwarn;i++)
    {
        free_and_null(warnings[i].filename);
        free_and_null(warnings[i].rawerrdata);
        free_and_null(warnings[i].fullerrdata);
        free_and_null(warnings[i].callerfilename);
        free_and_null(warnings[i].block);
    }
    warnings.reset();
    numwarn=0;

    for (int i=0;i<labelsinldata;i++) free((void*)ldata[i].name);
    ldata.reset();
    labelsinldata=0;

    for (int i=0;i<definesinddata;i++)
    {
        free((void*)ddata[i].name);
        free((void*)ddata[i].contents);
    }
    ddata.reset();
    definesinddata=0;
#undef free_and_null

    romCrc = 0;
    clidefines.reset();
    reset_warnings_to_default();

    reseteverything();
}

#define maxromsize (16*1024*1024)

static bool expectsNewAPI = false;

struct patchparams_v160 : public patchparams_base
{
    const char * patchloc;
    char * romdata;
    int buflen;
    int * romlen;

    const char** includepaths;
    int numincludepaths;

    bool should_reset;

    const definedata* additional_defines;
    int definecount;

    const char* stdincludesfile;
    const char* stddefinesfile;

    const warnsetting * warning_settings;
    int warning_setting_count;

    const struct memoryfile * memory_files;
    int memory_file_count;

    bool override_checksum_gen;
    bool generate_checksum;
};

struct patchparams : public patchparams_v160
{

};

static void asar_patch_begin(char * romdata_, int buflen, int * romlen_, bool should_reset)
{
    if (buflen != maxromsize)
    {
        romdata_r = (unsigned char*)malloc(maxromsize);
        memcpy(const_cast<unsigned char*>(romdata_r)/*we just allocated this, it's safe to violate its const*/, romdata_, (size_t)*romlen_);
    }
    else romdata_r = (unsigned char*)romdata_;
    romdata = (unsigned char*)malloc(maxromsize);
    // RPG Hacker: Without this memset, freespace commands can (and probably will) fail.
    memset((void*)romdata, 0, maxromsize);
    memcpy(const_cast<unsigned char*>(romdata), romdata_, (size_t)*romlen_);
    if (should_reset)
        resetdllstuff();
    romlen = *romlen_;
    romlen_r = *romlen_;
}

static void asar_patch_main(const char * patchloc)
{
    if (!path_is_absolute(patchloc)) asar_throw_warning(pass, warning_id_relative_path_used, "patch file");

    try
    {
        for (pass = 0;pass < 3;pass++)
        {
            initstuff();
            assemblefile(patchloc, true);
            finishpass();
        }
    }
    catch (errfatal&) {}
}

static bool asar_patch_end(char * romdata_, int buflen, int * romlen_)
{
    if (checksum_fix_enabled) fixchecksum();
    if (romdata_ != (const char*)romdata_r) free(const_cast<unsigned char*>(romdata_r));
    if (buflen < romlen) asar_throw_error(pass, error_type_null, error_id_buffer_too_small);
    if (errored)
    {
        free(const_cast<unsigned char*>(romdata));
        return false;
    }
    if (*romlen_ != buflen)
    {
        *romlen_ = romlen;
    }
    romCrc = crc32((const uint8_t*)romdata, (size_t)romlen);
    memcpy(romdata_, romdata, (size_t)romlen);
    free(const_cast<unsigned char*>(romdata));
    return true;
}

namespace asar {

    static void addlabel(const string & name, const snes_label & label_data)
    {
        labeldata label;
        label.name = strdup(name);
        label.location = (int)(label_data.pos & 0xFFFFFF);
        ldata[labelsinldata++] = label;
    }

    /*
     * Exports
    */
    #undef snestopc
    export int snestopc_pick(int addr)
    {
	    return snestopc(addr);
    }

    export bool asar_init()
    {
        if (!expectsNewAPI) return false;
        return true;
    }

    export int asar_version()
    {
        return get_version_int();
    }

    export int asar_apiversion()
    {
        expectsNewAPI=true;
        return 303;
    }

    export bool asar_reset()
    {
        resetdllstuff();
        pass=0;
        return true;
    }

    export void asar_close()
    {
        resetdllstuff();
    }

    export bool asar_patch(const char *patchloc, char *romdata_, int buflen, int *romlen_)
    {
        auto execute_patch = [&]() {
            asar_patch_begin(romdata_, buflen, romlen_, true);

            virtual_filesystem new_filesystem;
            new_filesystem.initialize(nullptr, 0);
            filesystem = &new_filesystem;

            asar_patch_main(patchloc);

            new_filesystem.destroy();
            filesystem = nullptr;

            return asar_patch_end(romdata_, buflen, romlen_);
        };
    #if defined(RUN_VIA_FIBER)
        return run_as_fiber(execute_patch);
    #else
        return execute_patch();
    #endif
    }

    export bool asar_patch_ex(const patchparams_base *params)
    {
        auto execute_patch = [&]() {
            if (params == nullptr)
            {
                asar_throw_error(pass, error_type_null, error_id_params_null);
            }

            if (params->structsize != sizeof(patchparams_v160))
            {
                asar_throw_error(pass, error_type_null, error_id_params_invalid_size);
            }

            patchparams paramscurrent;
            memset(&paramscurrent, 0, sizeof(paramscurrent));
            memcpy(&paramscurrent, params, (size_t)params->structsize);


            asar_patch_begin(paramscurrent.romdata, paramscurrent.buflen, paramscurrent.romlen, paramscurrent.should_reset);

            autoarray<string> includepaths;
            autoarray<const char*> includepath_cstrs;

            for (int i = 0; i < paramscurrent.numincludepaths; ++i)
            {
                if (!path_is_absolute(paramscurrent.includepaths[i])) asar_throw_warning(pass, warning_id_relative_path_used, "include search");
                string& newpath = includepaths.append(paramscurrent.includepaths[i]);
                includepath_cstrs.append((const char*)newpath);
            }

            if (paramscurrent.stdincludesfile != nullptr) {
                if (!path_is_absolute(paramscurrent.stdincludesfile)) asar_throw_warning(pass, warning_id_relative_path_used, "std includes file");
                string stdincludespath = paramscurrent.stdincludesfile;
                parse_std_includes(stdincludespath, includepaths);
            }

            for (int i = 0; i < includepaths.count; ++i)
            {
                includepath_cstrs.append((const char*)includepaths[i]);
            }

            size_t includepath_count = (size_t)includepath_cstrs.count;
            virtual_filesystem new_filesystem;
            new_filesystem.initialize(&includepath_cstrs[0], includepath_count);
            filesystem = &new_filesystem;

            for(int i = 0; i < paramscurrent.memory_file_count; ++i) {
                memoryfile f = paramscurrent.memory_files[i];
                filesystem->add_memory_file(f.path, f.buffer, f.length);
            }

            clidefines.reset();
            for (int i = 0; i < paramscurrent.definecount; ++i)
            {
                string name = (paramscurrent.additional_defines[i].name != nullptr ? paramscurrent.additional_defines[i].name : "");
                name = strip_whitespace(name);
                name = strip_prefix(name, '!', false); // remove leading ! if present
                if (!validatedefinename(name)) asar_throw_error(pass, error_type_null, error_id_cmdl_define_invalid, "asar_patch_ex() additional defines", name.data());
                if (clidefines.exists(name)) {
                    asar_throw_error(pass, error_type_null, error_id_cmdl_define_override, "asar_patch_ex() additional define", name.data());
                    return false;
                }
                string contents = (paramscurrent.additional_defines[i].contents != nullptr ? paramscurrent.additional_defines[i].contents : "");
                clidefines.create(name) = contents;
            }

            if (paramscurrent.stddefinesfile != nullptr) {
                if (!path_is_absolute(paramscurrent.stddefinesfile)) asar_throw_warning(pass, warning_id_relative_path_used, "std defines file");
                string stddefinespath = paramscurrent.stddefinesfile;
                parse_std_defines(stddefinespath);
            } else {
                parse_std_defines(nullptr); // needed to populate builtin defines
            }

            for (int i = 0; i < paramscurrent.warning_setting_count; ++i)
            {
                asar_warning_id warnid = parse_warning_id_from_string(paramscurrent.warning_settings[i].warnid, pass);

                if (warnid != warning_id_end)
                {
                    set_warning_enabled(warnid, paramscurrent.warning_settings[i].enabled);
                }
                else
                {
                    asar_throw_error(pass, error_type_null, error_id_invalid_warning_id, "asar_patch_ex() warning_settings", (int)(warning_id_start + 1), (int)(warning_id_end - 1));
                }
            }

            if(paramscurrent.override_checksum_gen) {
                checksum_fix_enabled = paramscurrent.generate_checksum;
                force_checksum_fix = true;
            }

            asar_patch_main(paramscurrent.patchloc);

            closecachedfiles(); // this needs the vfs so do it before destroying it
            new_filesystem.destroy();
            filesystem = nullptr;

            return asar_patch_end(paramscurrent.romdata, paramscurrent.buflen, paramscurrent.romlen);
    };
    #if defined(RUN_VIA_FIBER)
        return run_as_fiber(execute_patch);
    #else
        return execute_patch();
    #endif
    }

    export int asar_maxromsize()
    {
        return maxromsize;
    }

    export const errordata * asar_geterrors(int * count)
    {
        *count=numerror;
        return errors;
    }

    export const errordata * asar_getwarnings(int * count)
    {
        *count=numwarn;
        return warnings;
    }

    export const char * const * asar_getprints(int * count)
    {
        *count=numprint;
        return prints;
    }

    export const labeldata * asar_getalllabels(int * count)
    {
        for (int i=0;i<labelsinldata;i++) free((void*)ldata[i].name);
        labelsinldata=0;
        labels.each(addlabel);
        *count=labelsinldata;
        return ldata;
    }

    export int asar_getlabelval(const char * name)
    {
        if (!stricmp(name, ":$:opcodes:$:")) return numopcodes;//aaah, you found me
        int i;
        try {
            i=(int)labelval(&name).pos;
        }
        catch(errfatal&) { return -1; }
        if (*name || i<0) return -1;
        else return i&0xFFFFFF;
    }

    export const char * asar_getdefine(const char * name)
    {
        if (!defines.exists(name)) return "";
        return defines.find(name);
    }

    export const char * asar_resolvedefines(const char * data)
    {
        static string out;
        out = "";
        try
        {
            resolvedefines(out, data);
        }
        catch(errfatal&){
            out = "";
        }
        return out;
    }

    static void adddef(const string& name, string& value)
    {
        definedata define;
        define.name= duplicate_string(name);
        define.contents= duplicate_string(value);
        ddata[definesinddata++]=define;
    }

    export const definedata * asar_getalldefines(int * count)
    {
        for (int i=0;i<definesinddata;i++)
        {
            free((void*)ddata[i].name);
            free((void*)ddata[i].contents);
        }
        definesinddata=0;
        defines.each(adddef);
        *count=definesinddata;
        return ddata;
    }

    export double asar_math(const char * str, const char ** e)
    {
        ns="";
        namespace_list.reset();
        sublabels.reset();
        errored=false;
        ismath=true;
        initmathcore();
        double rval=0;
        try
        {
            rval=(double)math(str);
        }
        catch(errfatal&)
        {
            *e=matherror;
        }
        ismath=false;
        deinitmathcore();
        return rval;
    }

    export const writtenblockdata * asar_getwrittenblocks(int * count)
    {
        *count = writtenblocks.count;
        return writtenblocks;
    }

    export mapper_t asar_getmapper()
    {
        return mapper;
    }

    export const char * asar_getsymbolsfile(const char* type){
        symbolsfile = create_symbols_file(type, romCrc);
        return symbolsfile;
    }

}

#pragma warning(pop)