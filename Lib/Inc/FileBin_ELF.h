/**
 *  \file       FileBin_ELF.h
 *  \brief      ELF binary format parser
 *
 *  \version    1.0
 *  \date       Jun 12, 2016
 *  \author     Xavier Descarrega - DEVE embedded designs <info@deve.tech>
 *
 *  \copyright  MIT License
 *
 *              Copyright (c) 2016 Xavier Descarrega
 *
 *              Permission is hereby granted, free of charge, to any person obtaining a copy
 *              of this software and associated documentation files (the "Software"), to deal
 *              in the Software without restriction, including without limitation the rights
 *              to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *              copies of the Software, and to permit persons to whom the Software is
 *              furnished to do so, subject to the following conditions:
 *
 *              The above copyright notice and this permission notice shall be included in all
 *              copies or substantial portions of the Software.
 *
 *              THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *              IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *              FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *              AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
 *              LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *              OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *              SOFTWARE.
 *
 */

#ifndef FILEBIN_ELF_H
#define FILEBIN_ELF_H

extern "C" {
    #include "FileBin_ELF_Def.h"
}
#include <vector>
#include <string>
#include <cstdint>  // for uint32_t, uint16_t, etc.
#include <cstdio>   // for FILE*

typedef struct
{
    std::string Name;
} SectionInfoType;

struct SectionMapEntry {
    uint32_t vaStart;    // section virtual address
    uint32_t vaEnd;      // vaStart + sh_size
    uint32_t fileOffset; // sh_offset in ELF file
};

class FileBin_ELF
{

    private:

        FILE* file_descriptor = nullptr;
        std::string File_Name;
        Elf32_Ehdr elf_header;
        bool debugInfoAbbrevFound = false;
        bool debugInfoInfoFound = false;
        bool debugInfoStrFound = false;
        uint32_t AbbrevOffset = 0, AbbrevLen = 0, InfoOffset = 0, StrOffset = 0, InfoLen = 0;
        std::vector<SectionInfoType> SectionNameStr;
        Elf32_Shdr string_section;
        const Elf32_Shdr *symbol_section_header, *symbol_string_section_header;
        std::vector<SectionMapEntry> sectionMap;

    public:

        std::vector<SectionMapEntry> buildSectionMap(const std::vector<Elf32_Shdr>& section_headers);
        const SectionMapEntry* findSectionForVA( uint32_t va);

        std::vector<uint8_t> readSymbolFromELF(uint32_t symbolVA,
                                               uint32_t symbolSize);
        FileBin_ELF(void);
        uint8_t Parse(const std::string& file_name);
        std::string GetStr_ElfClass(const Elf32_Ehdr* elf_header) const;
        std::string GetStr_ElfDataEncoding(const Elf32_Ehdr* elf_header) const;
        std::string GetStr_ElfOSABI(const Elf32_Ehdr* elf_header) const;
        std::string GetStr_ElfType(const Elf32_Ehdr* elf_header) const;
        std::string GetStr_ElfMachine(const Elf32_Ehdr* elf_header) const;
        std::string GetStr_SectionHeader(const Elf32_Shdr* section_header) const;
        bool IsDWARF(void) const;
        uint32_t GetAbbrevOffset(void) const;
        uint32_t GetAbbrevLen(void) const;
        uint32_t GetInfoOffset(void) const;
        uint32_t GetInfoLen(void) const;
        uint32_t GetStrOffset(void) const;
        uint32_t GetRODataOffset(void) const;
        void PrintElfHeader(const Elf32_Ehdr* elf_header) const;

        void Print(void) const;
};

#endif // FILEBIN_ELF_H
