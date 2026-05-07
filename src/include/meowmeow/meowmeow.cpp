/*
 *  meOWmeOW
 *  (c) 2025 Arinsu (Ari)
*/
#include "meowmeow.h"

namespace meOWmeOW {

    bool meowmeow::init_meowmeow(Rom& rom) {
        // Verify if there is an extra byte table already in ROM.
        rom.old_extra_bytes = new char[0x80] { 0x03 };

        const int extra_byte_loc = ow_rev ? snestopc_pick(rom.rom_mapper, rom.read<3>(LVL_SPRITE_EXTRA_BYTES_PTR, true)) + HEADER_SIZE + EXTRA_BIT_OFFSET\
            : snestopc_pick(rom.rom_mapper, rom.read<3>(OW_SPRITE_EXTRA_BYTES_PTR, true)) + HEADER_SIZE;
        const bool extra_bytes_enabled = ow_rev ? rom.read<1>(LVL_SPRITE_EXTRA_BYTES_PTR + 3) == 0x42\
            : rom.read<1>(OW_SPRITE_EXTRA_BYTES_PTR + 3) == 0x42;

        if (extra_bytes_enabled)
        {
            // Extra byte table exists, acquire it from the ROM.
            rom.rom_data.clear();
            rom.rom_data.seekg(extra_byte_loc);
            rom.rom_data.read(rom.old_extra_bytes, 0x80);

            if (rom.rom_data.rdstate() != ios::goodbit)
                return false;
        }
        else
        {
            // Extra byte table does not exist, initialize it to 0x04 for backwards compatibility (three bytes for number/position + 1 extra)
            for (int i = 1; i < 0x80; ++i)
                rom.old_extra_bytes[i] = 0x04;
        }

        return true;
    }

    bool meowmeow::execute_meowmeow(Rom& rom, std::string tool_folder, std::vector<uint8_t>& new_sprite_data) {
        // LM versions older than 3.51 don't need meOWmeOW
        if(lm_ver < 351)
            return false;

        // Verify whether we need meOWmeOW, by any extra byte changes
        bool run_meowmeow = false;
        bool lm351_offset = lm_ver < 360;

        if (ow_rev || !( rom.read<3>(OW_SPRITE_DATA_PTR) == 0xFFFFFF || rom.read<3>(OW_SPRITE_DATA_PTR) == 0x000000 ) )
        {
            for (int i = 0; i < 0x7F; ++i)
            {
                if (rom.old_extra_bytes[i + (lm351_offset|ow_rev)] != rom.new_extra_bytes[i])
                {
                    run_meowmeow = true;
                    break;
                }
            }
        }

        if (run_meowmeow)
        {
            fmt::println("Extra byte changes detected. Running meOWmeOW to align data.");
            return ow_rev ? exec_meowmeow_lvl(rom, tool_folder, new_sprite_data, lm351_offset) : exec_meowmeow_ow(rom, tool_folder, new_sprite_data, lm351_offset);
        }
        return true;
    }

    bool meowmeow::exec_meowmeow_lvl(Rom& rom, std::string tool_folder, std::vector<uint8_t>& new_sprite_data, bool lm351_offset) {
        char* old_level_extra_bytes = new char[0x400] {};
        rom.rom_data.clear();
        rom.rom_data.seekg(snestopc_pick(rom.rom_mapper, rom.read<3>(LVL_SPRITE_EXTRA_BYTES_PTR, true)) + HEADER_SIZE);
        rom.rom_data.read(old_level_extra_bytes, 0x400);

        const int first_map = rom.read<2>(OWREV_FIRST_MAP_LVL, true);
        const int submaps = rom.read<2>(OWREV_SUBMAPS, true);

        std::string meowmeow_patch;

        for (int submaps_processed = 0; submaps_processed <= submaps; ++submaps_processed)
        {
            int curr_map = first_map + submaps_processed;
            // Prepare ROM data by putting the needle in the first map's data.
            rom.rom_data.clear();
            rom.rom_data.seekg(snestopc_pick(rom.rom_mapper, (rom.read<2>(LVL_SPRITE_DATA_PTR + (curr_map * 2), true)) | \
                                                         (rom.read<1>(LVL_SPRITE_DATA_PTR_BANK + curr_map) << 16)) + HEADER_SIZE);

            // Sprite header - see https://smwspeedruns.com/Level_Data_Format#Sprite_Header
            uint8_t sprite_header = rom.rom_data.get();
            bool exlevel = sprite_header & 0x20;

            new_sprite_data.push_back(sprite_header);
            // Sprite data - see https://smwspeedruns.com/Level_Data_Format#Sprite_Data
            while (1)
            {
                uint8_t sprite_yyyyEESY = rom.rom_data.get();
                new_sprite_data.push_back(sprite_yyyyEESY);

                uint8_t sprite_XXXXssss;
                if (sprite_yyyyEESY == 0xFF)
                {
                    // FF byte - if this isn't a big level, it's the terminator, we're out
                    if (!exlevel)
                        break;

                    // This is a big level. Next byte being FE is the terminator in this case
                    sprite_XXXXssss = rom.rom_data.get();
                    new_sprite_data.push_back(sprite_XXXXssss);
                    if (sprite_XXXXssss == 0xFE)
                        break;

                    // Not FE, then this is a position jump and the next byte is yyyyEESY again.
                }
                else
                {
                    // No FF byte
                    sprite_XXXXssss = rom.rom_data.get();
                    new_sprite_data.push_back(sprite_XXXXssss);

                    // Third always present byte is the sprite number
                    uint8_t sprite_NNNNNNNN = rom.rom_data.get();
                    new_sprite_data.push_back(sprite_NNNNNNNN);

                    auto true_ow_sprite_num = sprite_NNNNNNNN + ((sprite_yyyyEESY & 0x0C) << 6);
                    if (((sprite_yyyyEESY & 0x0C) == 0x04) && (sprite_NNNNNNNN != 0x00) && (sprite_NNNNNNNN < 0x80))
                    {
                        // Well-formed OWRev overworld sprite (extra bit 1 and between 01 and 7F)
                        int old_extra = rom.old_extra_bytes[sprite_NNNNNNNN] - 0x03;
                        int new_extra = rom.new_extra_bytes[sprite_NNNNNNNN - 1] - 0x03;

                        if (old_extra <= new_extra)
                        {
                            // The same amount of extra bytes: copy the same data
                            for (int i = 0; i < old_extra; ++i)
                                new_sprite_data.push_back((uint8_t)(rom.rom_data.get()));

                            // More extra bytes: fill new with 00
                            for (int i = old_extra; i < new_extra; ++i)
                                new_sprite_data.push_back(0x00);
                        }
                        else
                        {
                            // Less extra bytes: only copy the required ones
                            for (int i = 0; i < new_extra; ++i)
                                new_sprite_data.push_back((uint8_t)(rom.rom_data.get()));

                            // Discard the rest of the old extra bytes
                            for (int i = new_extra; i < old_extra; ++i)
                                rom.rom_data.get();
                        }

                    }
                    else
                    {
                        // Ill-formed sprite. Still, we gotta proof for whatever edited OWRev code someone is using.
                        // ...or for idiots.
                        int extra = old_level_extra_bytes[true_ow_sprite_num] - 0x03;
                        for (int i = 0; i < extra; ++i)
                            new_sprite_data.push_back((uint8_t)(rom.rom_data.get()));
                    }

                }

                if (new_sprite_data.size() > SPRITE_SIZE_LIMIT)
                    return false;
            }

            // Save table of corrected sprites per map level
            std::ofstream(tool_folder + "asm/new_sprite_data.bin", ios::binary).write((char*)new_sprite_data.data(), new_sprite_data.size());

            // Generate patch per level
            meowmeow_patch = std::format("\
if read1($00FFD5) == $23\n\
    if read1($00FFD7) == $0D\n\
        fullsa1rom\n\
    else\n\
        sa1rom\n\
    endif\n\
else\n\
    lorom\n\
endif\n\
autoclean read2(${0:0>6X}+(2*${2:0>4X}))|(read1(${1:0>6X}+${2:0>4X})<<16)\n\
org ${0:0>6X}+(2*${2:0>4X})\n\
    dw new_sprite_data\n\
org ${1:0>6X}+${2:0>4X}\n\
    db bank(new_sprite_data)\n\n\
freedata cleaned\n\
new_sprite_data:\n\
    incbin \"{3}asm/new_sprite_data.bin\"", LVL_SPRITE_DATA_PTR, LVL_SPRITE_DATA_PTR_BANK,
                curr_map, tool_folder);

            if (!rom.inline_patch(tool_folder, meowmeow_patch.c_str()))
                return false;

            new_sprite_data.assign({});
        }

        return true;
    }

    bool meowmeow::exec_meowmeow_ow(Rom& rom, std::string tool_folder, std::vector<uint8_t>& new_sprite_data, bool lm351_offset) {
        const int ow_sprite_data = rom.read<3>(OW_SPRITE_DATA_PTR, true);
        int submap_offset[OW_SUBMAPS] = { static_cast<int>(rom.read<2>(ow_sprite_data, true)),
                                        static_cast<int>(rom.read<2>(ow_sprite_data + 2, true)),
                                        static_cast<int>(rom.read<2>(ow_sprite_data + 4, true)),
                                        static_cast<int>(rom.read<2>(ow_sprite_data + 6, true)),
                                        static_cast<int>(rom.read<2>(ow_sprite_data + 8, true)),
                                        static_cast<int>(rom.read<2>(ow_sprite_data + 10, true)),
                                        static_cast<int>(rom.read<2>(ow_sprite_data + 12, true)) };

        int submaps_processed = 0;
        int bytes_processed = 0;

        // Prepare ROM data by putting the needle in the first map's data.
        rom.rom_data.clear();
        rom.rom_data.seekg(snestopc_pick(rom.rom_mapper, ow_sprite_data + submap_offset[0]) + HEADER_SIZE);
        while (1)
        {
            // These two are always present: they define a sprite number and its position.
            uint8_t sprite_xnnnnnnn = rom.rom_data.get();
            uint8_t sprite_yyyxxxxx = rom.rom_data.get();

            new_sprite_data.push_back(sprite_xnnnnnnn);
            new_sprite_data.push_back(sprite_yyyxxxxx);
            bytes_processed += 2;

            if (sprite_xnnnnnnn == 0x00 && sprite_yyyxxxxx == 0x00)
            {
                // Stop when all maps have been processed.
                if (++submaps_processed == OW_SUBMAPS)
                    break;

                // Move to next map's sprite data.
                rom.rom_data.clear();
                rom.rom_data.seekg(snestopc_pick(rom.rom_mapper, ow_sprite_data + submap_offset[submaps_processed]) + HEADER_SIZE);

                // Store the new offset for new map and move on.
                // The very first map (Main Map in LM) will *always* be 0x000E -- notice this is OW_SUBMAPS*2.
                submap_offset[submaps_processed] = bytes_processed + (OW_SUBMAPS * 2);
                continue;
            }

            // Save third always present byte.
            uint8_t sprite_zzzzzyyy = rom.rom_data.get();
            new_sprite_data.push_back(sprite_zzzzzyyy);

            // Verify if this is a sprite which changed.
            int sprite_number = sprite_xnnnnnnn & 0x7F;
            int old_extra = rom.old_extra_bytes[sprite_number - !lm351_offset] - 0x03;
            int new_extra = rom.new_extra_bytes[sprite_number - 1] - 0x03;

            if (old_extra <= new_extra)
            {
                // The same amount of extra bytes: copy the same data
                for (int i = 0; i < old_extra; ++i)
                    new_sprite_data.push_back((uint8_t)(rom.rom_data.get()));

                // More extra bytes: fill new with 00
                for (int i = old_extra; i < new_extra; ++i)
                    new_sprite_data.push_back(0x00);
            }
            else
            {
                // Less extra bytes: only copy the required ones
                for (int i = 0; i < new_extra; ++i)
                    new_sprite_data.push_back((uint8_t)(rom.rom_data.get()));

                // Discard the rest of the old extra bytes
                for (int i = new_extra; i < old_extra; ++i)
                    rom.rom_data.get();
            }

            bytes_processed += new_extra + 1;
        }

        // Save table of corrected sprites
        std::ofstream(tool_folder + "asm/new_sprite_data.bin", ios::binary).write((char*)new_sprite_data.data(), new_sprite_data.size());

        std::string meowmeow_patch = std::format("\
if read1($00FFD5) == $23\n\
    if read1($00FFD7) == $0D\n\
        fullsa1rom\n\
    else\n\
        sa1rom\n\
    endif\n\
else\n\
    lorom\n\
endif\n\
autoclean read3(${0:0>6X})\n\
org ${0:0>6X}\n\
    autoclean dl new_sprite_data\n\n\
freedata\n\
new_sprite_data:\n\
    dw ${1:0>4X}\n\
    dw ${2:0>4X}\n\
    dw ${3:0>4X}\n\
    dw ${4:0>4X}\n\
    dw ${5:0>4X}\n\
    dw ${6:0>4X}\n\
    dw ${7:0>4X}\n\n\
    incbin \"{8}asm/new_sprite_data.bin\"", OW_SPRITE_DATA_PTR, submap_offset[0],
            submap_offset[1],
            submap_offset[2],
            submap_offset[3],
            submap_offset[4],
            submap_offset[5],
            submap_offset[6],
            tool_folder);

        return rom.inline_patch(tool_folder, meowmeow_patch.c_str());

    }
}