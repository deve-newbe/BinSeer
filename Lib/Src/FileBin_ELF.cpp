/**
 *  \file       FileBin_ELF.cpp
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

#include "FileBin_ELF.h"
#include "FileBin_ELF_Def.h"
#include <algorithm>
#include <iostream>
#include <iomanip>    // for std::setw and std::setfill

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

using namespace std;

class MappedFile
{
public:
    const uint8_t* data = nullptr;
    size_t size = 0;

#if defined(_WIN32) || defined(_WIN64)
private:
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
#else
private:
    int fd = -1;
#endif

public:
    bool open(const std::string& fileName)
    {
#if defined(_WIN32) || defined(_WIN64)
        hFile = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        LARGE_INTEGER fsize;
        if (!GetFileSizeEx(hFile, &fsize)) return false;
        size = static_cast<size_t>(fsize.QuadPart);

        hMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (!hMap) return false;

        data = static_cast<const uint8_t*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
        if (!data) return false;

#else
        fd = ::open(fileName.c_str(), O_RDONLY);
        if (fd < 0) return false;

        struct stat st;
        if (fstat(fd, &st) < 0) return false;
        size = st.st_size;

        data = static_cast<const uint8_t*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (data == MAP_FAILED) return false;
#endif
        return true;
    }

    void close()
    {
#if defined(_WIN32) || defined(_WIN64)
        if (data) UnmapViewOfFile(data);
        if (hMap) CloseHandle(hMap);
        if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
#else
        if (data) munmap((void*)data, size);
        if (fd >= 0) ::close(fd);
#endif
        data = nullptr;
        size = 0;
    }

    ~MappedFile()
    {
        close();
    }
};

FileBin_ELF::FileBin_ELF()
{
}

std::string FileBin_ELF::GetStr_ElfClass(const Elf32_Ehdr *elf_header) const
{
    std::string elf_header_ident_class_str;
    switch (elf_header->e_ident[EI_CLASS])
    {
        case ELFCLASS32 : elf_header_ident_class_str = "32-bit"; break;
        case ELFCLASS64 : elf_header_ident_class_str = "64-bit"; break;
        default         : elf_header_ident_class_str = "Invalid object length"; break;
    }
    return elf_header_ident_class_str;
}

std::string FileBin_ELF::GetStr_ElfDataEncoding(const Elf32_Ehdr *elf_header) const
{
    std::string elf_header_ident_data_str;
    switch (elf_header->e_ident[EI_DATA])
    {
        case ELFDATA2LSB : elf_header_ident_data_str = "2's complement, little endian"; break;
        case ELFDATA2MSB : elf_header_ident_data_str = "2's complement, big endian"; break;
        default          : elf_header_ident_data_str = "Invalid data encoding"; break;
    }
    return elf_header_ident_data_str;
}

std::string FileBin_ELF::GetStr_ElfOSABI(const Elf32_Ehdr *elf_header) const
{
    std::string elf_header_ident_osabi_str;
    switch (elf_header->e_ident[EI_OSABI])
    {
        case ELFOSABI_SYSV          : elf_header_ident_osabi_str = "UNIX System V"; break;
        case ELFOSABI_HPUX          : elf_header_ident_osabi_str = "HP-UX"; break;
        case ELFOSABI_NETBSD        : elf_header_ident_osabi_str = "NetBSD"; break;
        case ELFOSABI_GNU           : elf_header_ident_osabi_str = "GNU ELF"; break;
        case ELFOSABI_SOLARIS       : elf_header_ident_osabi_str = "Sun Solaris"; break;
        case ELFOSABI_AIX           : elf_header_ident_osabi_str = "IBM AIX"; break;
        case ELFOSABI_IRIX          : elf_header_ident_osabi_str = "SGI Irix"; break;
        case ELFOSABI_FREEBSD       : elf_header_ident_osabi_str = "FreeBSD"; break;
        case ELFOSABI_TRU64         : elf_header_ident_osabi_str = "Compaq TRU64 UNIX"; break;
        case ELFOSABI_MODESTO       : elf_header_ident_osabi_str = "Novell Modesto"; break;
        case ELFOSABI_OPENBSD       : elf_header_ident_osabi_str = "OpenBSD"; break;
        case ELFOSABI_ARM_AEABI     : elf_header_ident_osabi_str = "ARM EABI"; break;
        case ELFOSABI_ARM           : elf_header_ident_osabi_str = "ARM"; break;
        case ELFOSABI_STANDALONE    : elf_header_ident_osabi_str = "Standalone (embedded) application"; break;
        default                     : elf_header_ident_osabi_str = "Invalid OSABI"; break;
    }
    return elf_header_ident_osabi_str;
}

std::string FileBin_ELF::GetStr_ElfType(const Elf32_Ehdr *elf_header) const
{
    std::string elf_header_type_str;
    switch (elf_header->e_type)
    {
        case ET_NONE:   elf_header_type_str = "ET_NONE: No file type"; break;
        case ET_REL:    elf_header_type_str = "ET_REL: Relocatable file"; break;
        case ET_EXEC:   elf_header_type_str = "ET_EXEC: Executable file"; break;
        case ET_DYN:    elf_header_type_str = "ET_DYN: Shared object file"; break;
        case ET_CORE:   elf_header_type_str = "ET_CORE: Core file"; break;
        case ET_NUM:    elf_header_type_str = "ET_NUM: Number of defined types"; break;
        case ET_LOOS:   elf_header_type_str = "ET_LOOS: OS-specific range start"; break;
        case ET_HIOS:   elf_header_type_str = "ET_HIOS: OS-specific range end"; break;
        case ET_LOPROC: elf_header_type_str = "ET_LOPROC: Processor-specific range start"; break;
        case ET_HIPROC: elf_header_type_str = "ET_HIPROC: Processor-specific range end"; break;
        default:        elf_header_type_str = "Invalid Type"; break;
    }
    return elf_header_type_str;
}

std::string FileBin_ELF::GetStr_ElfMachine(const Elf32_Ehdr *elf_header) const
{
    std::string elf_header_machine_str ;
    switch (elf_header->e_machine)
    {
        case EM_NONE:           elf_header_machine_str = "No machine architecture"; break;
        case EM_M32:            elf_header_machine_str = "AT&T WE 32100 "; break;
        case EM_SPARC:          elf_header_machine_str = "SUN SPARC "; break;
        case EM_386:            elf_header_machine_str = "Intel 80386 "; break;
        case EM_68K:            elf_header_machine_str = "Motorola m68k family "; break;
        case EM_88K:            elf_header_machine_str = "Motorola m88k family "; break;
        case EM_860:            elf_header_machine_str = "Intel 80860 "; break;
        case EM_MIPS:           elf_header_machine_str = "MIPS R3000 big-endian "; break;
        case EM_S370:           elf_header_machine_str = "IBM System/370 "; break;
        case EM_MIPS_RS3_LE:    elf_header_machine_str = "MIPS R3000 little-endian "; break;
        case EM_PARISC:         elf_header_machine_str = "HPPA "; break;
        case EM_VPP500:         elf_header_machine_str = "Fujitsu VPP500 "; break;
        case EM_SPARC32PLUS:    elf_header_machine_str = "Sun's 'v8plus'"; break;
        case EM_960:            elf_header_machine_str = "Intel 80960 "; break;
        case EM_PPC:            elf_header_machine_str = "PowerPC "; break;
        case EM_PPC64:          elf_header_machine_str = "PowerPC 64-bit "; break;
        case EM_S390:           elf_header_machine_str = "IBM S390 "; break;
        case EM_V800:           elf_header_machine_str = "NEC V800 series "; break;
        case EM_FR20:           elf_header_machine_str = "Fujitsu FR20 "; break;
        case EM_RH32:           elf_header_machine_str = "TRW RH-32 "; break;
        case EM_RCE:            elf_header_machine_str = "Motorola RCE "; break;
        case EM_ARM:            elf_header_machine_str = "ARM "; break;
        case EM_FAKE_ALPHA:     elf_header_machine_str = "Digital Alpha "; break;
        case EM_SH:             elf_header_machine_str = "Hitachi SH "; break;
        case EM_SPARCV9:        elf_header_machine_str = "SPARC v9 64-bit "; break;
        case EM_TRICORE:        elf_header_machine_str = "Siemens/Infineon Tricore "; break;
        case EM_ARC:            elf_header_machine_str = "Argonaut RISC Core "; break;
        case EM_H8_300:         elf_header_machine_str = "Hitachi H8/300 "; break;
        case EM_H8_300H:        elf_header_machine_str = "Hitachi H8/300H "; break;
        case EM_H8S:            elf_header_machine_str = "Hitachi H8S "; break;
        case EM_H8_500:         elf_header_machine_str = "Hitachi H8/500 "; break;
        case EM_IA_64:          elf_header_machine_str = "Intel Merced "; break;
        case EM_MIPS_X:         elf_header_machine_str = "Stanford MIPS-X "; break;
        case EM_COLDFIRE:       elf_header_machine_str = "Motorola Coldfire "; break;
        case EM_68HC12:         elf_header_machine_str = "Motorola M68HC12 "; break;
        case EM_MMA:            elf_header_machine_str = "Fujitsu MMA Multimedia Accelerator"; break;
        case EM_PCP:            elf_header_machine_str = "Siemens PCP "; break;
        case EM_NCPU:           elf_header_machine_str = "Sony nCPU embeeded RISC "; break;
        case EM_NDR1:           elf_header_machine_str = "Denso NDR1 microprocessor "; break;
        case EM_STARCORE:       elf_header_machine_str = "Motorola Start*Core processor "; break;
        case EM_ME16:           elf_header_machine_str = "Toyota ME16 processor "; break;
        case EM_ST100:          elf_header_machine_str = "STMicroelectronic ST100 processor "; break;
        case EM_TINYJ:          elf_header_machine_str = "Advanced Logic Corp. Tinyj emb.fam"; break;
        case EM_X86_64:         elf_header_machine_str = "AMD x86-64 architecture "; break;
        case EM_PDSP:           elf_header_machine_str = "Sony DSP Processor "; break;
        case EM_FX66:           elf_header_machine_str = "Siemens FX66 microcontroller "; break;
        case EM_ST9PLUS:        elf_header_machine_str = "STMicroelectronics ST9+ 8/16 mc "; break;
        case EM_ST7:            elf_header_machine_str = "STmicroelectronics ST7 8 bit mc "; break;
        case EM_68HC16:         elf_header_machine_str = "Motorola MC68HC16 microcontroller "; break;
        case EM_68HC11:         elf_header_machine_str = "Motorola MC68HC11 microcontroller "; break;
        case EM_68HC08:         elf_header_machine_str = "Motorola MC68HC08 microcontroller "; break;
        case EM_68HC05:         elf_header_machine_str = "Motorola MC68HC05 microcontroller "; break;
        case EM_SVX:            elf_header_machine_str = "Silicon Graphics SVx "; break;
        case EM_ST19:           elf_header_machine_str = "STMicroelectronics ST19 8 bit mc "; break;
        case EM_VAX:            elf_header_machine_str = "Digital VAX "; break;
        case EM_CRIS:           elf_header_machine_str = "Axis Communications 32-bit embedded processor "; break;
        case EM_JAVELIN:        elf_header_machine_str = "Infineon Technologies 32-bit embedded processor "; break;
        case EM_FIREPATH:       elf_header_machine_str = "Element 14 64-bit DSP Processor "; break;
        case EM_ZSP:            elf_header_machine_str = "LSI Logic 16-bit DSP Processor "; break;
        case EM_MMIX:           elf_header_machine_str = "Donald Knuth's educational 64-bit processor "; break;
        case EM_HUANY:          elf_header_machine_str = "Harvard University machine-independent object files "; break;
        case EM_PRISM:          elf_header_machine_str = "SiTera Prism "; break;
        case EM_AVR:            elf_header_machine_str = "Atmel AVR 8-bit microcontroller "; break;
        case EM_FR30:           elf_header_machine_str = "Fujitsu FR30 "; break;
        case EM_D10V:           elf_header_machine_str = "Mitsubishi D10V "; break;
        case EM_D30V:           elf_header_machine_str = "Mitsubishi D30V "; break;
        case EM_V850:           elf_header_machine_str = "NEC v850 "; break;
        case EM_M32R:           elf_header_machine_str = "Mitsubishi M32R "; break;
        case EM_MN10300:        elf_header_machine_str = "Matsushita MN10300 "; break;
        case EM_MN10200:        elf_header_machine_str = "Matsushita MN10200 "; break;
        case EM_PJ:             elf_header_machine_str = "picoJava "; break;
        case EM_OPENRISC:       elf_header_machine_str = "OpenRISC 32-bit embedded processor "; break;
        case EM_ARC_A5:         elf_header_machine_str = "ARC Cores Tangent-A5 "; break;
        case EM_XTENSA:         elf_header_machine_str = "Tensilica Xtensa Architecture "; break;
        case EM_ALTERA_NIOS2:   elf_header_machine_str = "Altera Nios II "; break;
        case EM_AARCH64:        elf_header_machine_str = "ARM AARCH64 "; break;
        case EM_TILEPRO:        elf_header_machine_str = "Tilera TILEPro "; break;
        case EM_MICROBLAZE:     elf_header_machine_str = "Xilinx MicroBlaze "; break;
        case EM_TILEGX:         elf_header_machine_str = "Tilera TILE-Gx "; break;
        default:                elf_header_machine_str = "Invalid Type"; break;
    }
    return elf_header_machine_str;
}

string FileBin_ELF::GetStr_SectionHeader(const Elf32_Shdr *section_header) const
{
    string section_header_type_str ;

    switch (section_header->sh_type)
    {
        case SHT_NULL:              section_header_type_str = "Section header table entry unused"; break;
        case SHT_PROGBITS:          section_header_type_str = "Program data "; break;
        case SHT_SYMTAB:            section_header_type_str = "Symbol table "; break;
        case SHT_STRTAB:            section_header_type_str = "String table "; break;
        case SHT_RELA:              section_header_type_str = "Relocation entries with addends "; break;
        case SHT_HASH:              section_header_type_str = "Symbol hash table "; break;
        case SHT_DYNAMIC:           section_header_type_str = "Dynamic linking information "; break;
        case SHT_NOTE:              section_header_type_str = "Notes "; break;
        case SHT_NOBITS:            section_header_type_str = "Program space with no data (bss) "; break;
        case SHT_REL:               section_header_type_str = "Relocation entries, no addends "; break;
        case SHT_SHLIB:             section_header_type_str = "Reserved "; break;
        case SHT_DYNSYM:            section_header_type_str = "Dynamic linker symbol table "; break;
        case SHT_INIT_ARRAY:        section_header_type_str = "Array of constructors "; break;
        case SHT_FINI_ARRAY:        section_header_type_str = "Array of destructors "; break;
        case SHT_PREINIT_ARRAY:     section_header_type_str = "Array of pre-constructors "; break;
        case SHT_GROUP:             section_header_type_str = " Section group "; break;
        case SHT_SYMTAB_SHNDX:      section_header_type_str = " Extended section indeces "; break;
        case SHT_NUM:               section_header_type_str = " Number of defined types.  "; break;
        case SHT_LOOS:              section_header_type_str = " Start OS-specific.  "; break;
        case SHT_GNU_ATTRIBUTES:    section_header_type_str = " Object attributes.  "; break;
        case SHT_GNU_HASH:          section_header_type_str = " GNU-style hash table.  "; break;
        case SHT_GNU_LIBLIST:       section_header_type_str = " Prelink library list "; break;
        case SHT_CHECKSUM:          section_header_type_str = " Checksum for DSO content.  "; break;
        case SHT_LOSUNW:            section_header_type_str = " Sun-specific low bound.  "; break;
        case SHT_SUNW_COMDAT:       section_header_type_str = ""; break;
        case SHT_SUNW_syminfo:      section_header_type_str = ""; break;
        case SHT_GNU_verdef:        section_header_type_str = " Version definition section.  "; break;
        case SHT_GNU_verneed:       section_header_type_str = " Version needs section.  "; break;
        case SHT_GNU_versym:        section_header_type_str = " Version symbol table.  "; break;
        case SHT_LOPROC:            section_header_type_str = " Start of processor-specific "; break;
        case SHT_HIPROC:            section_header_type_str = " End of processor-specific "; break;
        case SHT_LOUSER:            section_header_type_str = " Start of application-specific "; break;
        case SHT_HIUSER:            section_header_type_str = " End of application-specific "; break;
        default:                    section_header_type_str = "Invalid Section Type"; break;
    }

    return section_header_type_str;
}

void FileBin_ELF::PrintElfHeader(const Elf32_Ehdr *elf_header) const
{
    cout << "Class: " << GetStr_ElfClass(elf_header) << endl;               /* e_ident[EI_CLASS] */
    cout << "Encoding: " << GetStr_ElfDataEncoding(elf_header) << endl;     /* e_ident[EI_DATA] */
    cout << "OSABI: " << GetStr_ElfOSABI(elf_header) << endl;               /* e_ident[EI_OSABI] */
    cout << "Type: " << GetStr_ElfType(elf_header) << endl;                 /* e_type */
    cout << "Machine: " << GetStr_ElfMachine(elf_header) << endl;           /* e_machine */
    cout << "Version: " << elf_header->e_version << endl;
#if(0)
    ui->ui_elf_header_entry->setText("0x"+QString::number(elf_header->e_entry,16));
    ui->ui_elf_header_phoff->setText("0x"+QString::number(elf_header->e_phoff,16));
    ui->ui_elf_header_shoff->setText("0x"+QString::number(elf_header->e_shoff,16));
    ui->ui_elf_header_flags->setText(QString::number(elf_header->e_flags));
    ui->ui_elf_header_ehsize->setText(QString::number(elf_header->e_ehsize));
    ui->ui_elf_header_phentsize->setText(QString::number(elf_header->e_phentsize));
    ui->ui_elf_header_phnum->setText(QString::number(elf_header->e_phnum));
    ui->ui_elf_header_shentsize->setText(QString::number(elf_header->e_shentsize));
    ui->ui_elf_header_shnum->setText(QString::number(elf_header->e_shnum));
    ui->ui_elf_header_shstrndx->setText(QString::number(elf_header->e_shstrndx));
#endif

}

/*

void ElfParser::ParseProgramHeaders(Elf32_Phdr *program_header, uint32_t size)
{
    for ( uint32_t i = 0 ; i < size ; i++ )
    {
        ui->ui_table_program_header->insertRow(i);
        ui->ui_table_program_header->setItem(i,0,new QTableWidgetItem(QString::number(program_header[i].p_type)));
        ui->ui_table_program_header->setItem(i,1,new QTableWidgetItem(QString::number(program_header[i].p_flags)));
        ui->ui_table_program_header->setItem(i,2,new QTableWidgetItem("0x"+QString::number(program_header[i].p_offset,16)));
        ui->ui_table_program_header->setItem(i,3,new QTableWidgetItem("0x"+QString::number(program_header[i].p_vaddr,16)));
        ui->ui_table_program_header->setItem(i,4,new QTableWidgetItem("0x"+QString::number(program_header[i].p_paddr,16)));
        ui->ui_table_program_header->setItem(i,5,new QTableWidgetItem(QString::number(program_header[i].p_filesz)));
        ui->ui_table_program_header->setItem(i,6,new QTableWidgetItem(QString::number(program_header[i].p_memsz)));
    }
}
*/

void FileBin_ELF::Print(void) const
{
    cout << "[INFO] Parsed file name: " << this->File_Name << endl;
    this->PrintElfHeader(&this->elf_header);
    cout << "Start of section headers: " << (int)this->elf_header.e_shoff << " (bytes into file)" << endl;

    if (this->debugInfoAbbrevFound && this->debugInfoInfoFound && this->debugInfoStrFound)
    {
        cout << ".debug_abbrev section found at " << std::hex << this->AbbrevOffset << "size: " << AbbrevLen << endl;
        cout << ".debug_info section found at " << std::hex << this->InfoOffset << endl;
        cout << ".debug_str section found at " << std::hex << this->StrOffset << endl;
    }

    cout << "Section header count: " <<  this->SectionNameStr.size() << endl;
     std::string section_header_type_str ;
    for (int i = 0; i < this->SectionNameStr.size(); i++)
    {

        cout << "[" << std::setfill(' ') << std::setw(3) <<(int)i << "] " << std::setw(20) << this->SectionNameStr.at(i).Name << " " << section_header_type_str   <<endl;
    }
}

std::vector<SectionMapEntry> FileBin_ELF::buildSectionMap(const std::vector<Elf32_Shdr>& section_headers) {
    std::vector<SectionMapEntry> map;
    for (const auto& sh : section_headers) {
        if (sh.sh_size == 0) continue;
        SectionMapEntry entry;
        entry.vaStart = sh.sh_addr;
        entry.vaEnd = sh.sh_addr + sh.sh_size;
        entry.fileOffset = sh.sh_offset;
        map.push_back(entry);
    }

    // Sort by start VA (usually already sorted)
    std::sort(map.begin(), map.end(), [](const SectionMapEntry& a, const SectionMapEntry& b) {
        return a.vaStart < b.vaStart;
    });

    return map;
}

const SectionMapEntry* FileBin_ELF::findSectionForVA(uint32_t va) {
    size_t left = 0;
    size_t right = this->sectionMap.size();

    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (va < this->sectionMap[mid].vaStart) {
            right = mid;
        } else if (va >= this->sectionMap[mid].vaEnd) {
            left = mid + 1;
        } else {
            return &this->sectionMap[mid]; // Found
        }
    }
    return nullptr; // Not found
}

std::vector<uint8_t> FileBin_ELF::readSymbolFromELF(
                                       uint32_t symbolVA,
                                       uint32_t symbolSize)
{

    this->file_descriptor = fopen(this->File_Name.c_str(), "rb");
    if (!this->file_descriptor)
    {
        std::cout << "[ERROR] Unable to open ELF file: " << this->File_Name << std::endl;
        //return 1;
    }

    const SectionMapEntry* sec = findSectionForVA(symbolVA);
    if (!sec) {
        std::cerr << "[ERROR] Symbol VA 0x" << std::hex << symbolVA << " not in any section\n";
        return {};
    }

    uint32_t fileOffset = sec->fileOffset + (symbolVA - sec->vaStart);
    std::vector<uint8_t> buffer(symbolSize);

    if (fseek(this->file_descriptor, fileOffset, SEEK_SET) != 0) {
        std::cerr << "[ERROR] fseek failed\n";
        return {};
    }

    if (fread(buffer.data(), 1, symbolSize, this->file_descriptor) != symbolSize) {
        std::cerr << "[ERROR] fread failed\n";
        return {};
    }

    fclose(file_descriptor);

    return buffer;
}

uint8_t FileBin_ELF::Parse(const std::string& file_name)
{
    /* Clear symbol list */
    this->SectionNameStr.clear();
    this->debugInfoAbbrevFound = false;
    this->debugInfoInfoFound = false;
    this->debugInfoStrFound = false;
    this->File_Name = file_name;

    // Open memory-mapped file
    MappedFile file;
    if (!file.open(file_name)) {
        std::cout << "[ERROR] Unable to open ELF file: " << file_name << std::endl;
        return 1;
    }

    if (file.size < sizeof(Elf32_Ehdr)) {
        std::cout << "[ERROR] File too small for ELF header" << std::endl;
        return 2;
    }

    // Read ELF header directly from mapped memory
    elf_header = *reinterpret_cast<const Elf32_Ehdr*>(file.data);

    // Validate ELF header
    if (elf_header.e_phnum * sizeof(Elf32_Phdr) + elf_header.e_phoff > file.size ||
        elf_header.e_shnum * sizeof(Elf32_Shdr) + elf_header.e_shoff > file.size)
    {
        std::cout << "[ERROR] Invalid ELF header offsets" << std::endl;
        return 3;
    }

    // Read program headers and section headers directly
    const Elf32_Phdr* program_header = reinterpret_cast<const Elf32_Phdr*>(file.data + elf_header.e_phoff);
    const Elf32_Shdr* section_header = reinterpret_cast<const Elf32_Shdr*>(file.data + elf_header.e_shoff);

    // Identify string section and symbol tables
    for (uint32_t i = 0; i < elf_header.e_shnum; ++i)
    {
        if (i == elf_header.e_shstrndx) {
            string_section = section_header[i];
        }
        else if (section_header[i].sh_type == SHT_STRTAB) {
            symbol_string_section_header = &section_header[i];
        }

        if (section_header[i].sh_type == SHT_SYMTAB) {
            symbol_section_header = &section_header[i];
        }
    }

    sectionMap = buildSectionMap(std::vector<Elf32_Shdr>(section_header, section_header + elf_header.e_shnum));

    // Load string table
    if (string_section.sh_offset + string_section.sh_size > file.size) {
        std::cerr << "[ERROR] Invalid string table offset/size" << std::endl;
        return 4;
    }

    const uint8_t* string_table = file.data + string_section.sh_offset;
    // Parse sections
    for (uint32_t i = 0; i < elf_header.e_shnum; ++i)
    {
        SectionInfoType sectionInfo;
        if (section_header[i].sh_type != SHT_NULL)
            sectionInfo.Name = reinterpret_cast<const char*>(string_table + section_header[i].sh_name);

        bool parseHeaderData = (section_header[i].sh_type == SHT_PROGBITS);
        SectionNameStr.push_back(sectionInfo);

        if (parseHeaderData)
        {
            const std::string& name = sectionInfo.Name;
            if (name == ".debug_abbrev") {
                debugInfoAbbrevFound = true;
                AbbrevOffset = section_header[i].sh_offset;
                AbbrevLen = section_header[i].sh_size;
            } else if (name == ".debug_info") {
                debugInfoInfoFound = true;
                InfoOffset = section_header[i].sh_offset;
                InfoLen = section_header[i].sh_size;
            } else if (name == ".debug_str") {
                debugInfoStrFound = true;
                StrOffset = section_header[i].sh_offset;
            }
        }
    }

    // The file remains mapped; file.data points to ELF contents
    // No need to fclose; the destructor of MappedFile handles unmapping

    return 0;
}


#if(0)
uint8_t FileBin_ELF::Parse(const std::string& file_name)
{

    /* Clear symbol list */
    this->SectionNameStr.clear();
    this->debugInfoAbbrevFound = false;
    this->debugInfoInfoFound = false;
    this->debugInfoStrFound = false;
    this->File_Name = file_name;
    // Open file

    this->file_descriptor = fopen(file_name.c_str(), "rb");
    if (!this->file_descriptor)
    {
        std::cout << "[ERROR] Unable to open ELF file: " << file_name << std::endl;
        return 1;
    }

    // Read ELF header
    if (fread(&this->elf_header, sizeof(this->elf_header), 1, this->file_descriptor) != 1)
    {
        std::cout << "[ERROR] Unable to read ELF header" << std::endl;
        fclose(this->file_descriptor);
        return 2;
    }

    // Read program headers
    std::vector<Elf32_Phdr> program_header(this->elf_header.e_phnum);
    if (fseek(file_descriptor, this->elf_header.e_phoff, SEEK_SET) != 0 ||
        fread(program_header.data(), sizeof(Elf32_Phdr), this->elf_header.e_phnum, file_descriptor) != this->elf_header.e_phnum)
    {
        std::cout << "[ERROR] Unable to read program headers" << std::endl;
        fclose(this->file_descriptor);
        return 3;
    }

    // Read section headers
    std::vector<Elf32_Shdr> section_header(this->elf_header.e_shnum);
    if (fseek(file_descriptor, this->elf_header.e_shoff, SEEK_SET) != 0 ||
        fread(section_header.data(), sizeof(Elf32_Shdr), this->elf_header.e_shnum, file_descriptor) != this->elf_header.e_shnum)
    {
        std::cout << "[ERROR] Unable to read section headers" << std::endl;
        fclose(this->file_descriptor);
        return 4;
    }

    // Identify string section and symbol tables
    for (uint32_t i = 0; i < this->elf_header.e_shnum; ++i)
    {
        if (i == this->elf_header.e_shstrndx)
        {
            this->string_section = section_header[i];
        }
        else if (section_header[i].sh_type == SHT_STRTAB)
        {
            this->symbol_string_section_header = &section_header[i];
        }

        if (section_header[i].sh_type == SHT_SYMTAB)
        {
            this->symbol_section_header = &section_header[i];
        }
    }

    this->sectionMap = buildSectionMap(section_header);

    // Load string table
    std::vector<uint8_t> string_table(this->string_section.sh_size);
    if (fseek(file_descriptor, this->string_section.sh_offset, SEEK_SET) != 0 ||
        fread(string_table.data(), 1, string_table.size(), file_descriptor) != string_table.size())
    {
        std::cout << "[ERROR] Unable to read string table" << std::endl;
        fclose(this->file_descriptor);
        return 5;
    }

    // Parse sections
    for (uint32_t i = 0; i < this->elf_header.e_shnum; ++i)
    {
        SectionInfoType sectionInfo;
        if (section_header[i].sh_type != SHT_NULL)
        {
            sectionInfo.Name = std::string(reinterpret_cast<const char*>(&string_table[section_header[i].sh_name]));
        }

        bool parseHeaderData = (section_header[i].sh_type == SHT_PROGBITS);
        SectionNameStr.push_back(sectionInfo);

        if (parseHeaderData)
        {
            const std::string& name = sectionInfo.Name;

            if (name == ".debug_abbrev")
            {
                debugInfoAbbrevFound = true;
                AbbrevOffset = section_header[i].sh_offset;
                AbbrevLen = section_header[i].sh_size;
            }
            else if (name == ".debug_info")
            {
                debugInfoInfoFound = true;
                InfoOffset = section_header[i].sh_offset;
                InfoLen = section_header[i].sh_size;
            }
            else if (name == ".debug_str")
            {
                debugInfoStrFound = true;
                StrOffset = section_header[i].sh_offset;
            }
        }

#if(0)
         //if (0 == strcmp(buffer2, ".debug_info"))


         break;

         // Parse symbols
         fseek(file_descriptor, symbol_string_section_header->sh_offset, SEEK_SET);

         char buffer[symbol_string_section_header->sh_size];

         fread(&buffer, 1, symbol_string_section_header->sh_size, file_descriptor);
         //for (int i = 0 ; i < 100 ; i++)
        // {
        //     cout << (int)i << " -> " << buffer[i] << endl;
        // }

         //cout << section_header[98].sh_name << endl;
         fseek(file_descriptor, symbol_section_header->sh_offset, SEEK_SET);

          uint32_t symbol_num = symbol_section_header->sh_size / sizeof(Elf32_Sym);

         Elf32_Sym symbol_table[symbol_num];

         fread(&symbol_table, 1, symbol_section_header->sh_size, file_descriptor);

         //char symbol_name[1024];

        //int index;

        SymbolElfType *NewSymbol;

        //cout << "symbol nums " << symbol_num << endl;

        for (uint32_t i = 0 ; i < symbol_num ; i++)
        {
            NewSymbol = new SymbolElfType();
           // index = static_cast<int>(i);
            //ui->ui_table_symbols->insertRow(index);
            //ui->ui_table_symbols->setItem(index,0,new QTableWidgetItem(QString::number(symbol_table[i].st_name)));
            strcpy(NewSymbol->Name, (char*)&buffer[symbol_table[i].st_name]);


            NewSymbol->Address = symbol_table[i].st_value;

           // cout << std::hex << NewSymbol->Address << " size: " << symbol_table[i].st_size << " -> " << NewSymbol->Name << endl;
    if (((ELF32_ST_BIND(symbol_table[i].st_info) == STB_GLOBAL)&&(ELF32_ST_TYPE(symbol_table[i].st_info) == STT_OBJECT))||((ELF32_ST_BIND(symbol_table[i].st_info) == STB_LOCAL)&&(ELF32_ST_TYPE(symbol_table[i].st_info) == STT_OBJECT)))
            ListSymbol->push(NewSymbol);
           // ui->ui_table_symbols->setItem(index,1,new QTableWidgetItem(QString::fromUtf8(symbol_name)));
    /*
            QString symbol_table_info_bind_str;
            switch(ELF32_ST_BIND(symbol_table[index].st_info))
            {
                case STB_LOCAL  : symbol_table_info_bind_str = "Local symbol"; break;
                case STB_GLOBAL : symbol_table_info_bind_str = "Global symbol"; break;
                case STB_WEAK   : symbol_table_info_bind_str = "Weak symbol"; break;
                case STB_NUM    : symbol_table_info_bind_str = "Number of defined types"; break;
                case STB_LOOS	: symbol_table_info_bind_str = "Start of OS-specific"; break;
                case STB_HIOS	: symbol_table_info_bind_str = "Unique symbol"; break;
                case STB_LOPROC	: symbol_table_info_bind_str = "End of OS-specific"; break;
                case STB_HIPROC	: symbol_table_info_bind_str = "Start of processor-specific"; break;
                default         : symbol_table_info_bind_str = "Invalid Bind"; break;
            }
            ui->ui_table_symbols->setItem(index,2, new QTableWidgetItem(symbol_table_info_bind_str));

            QString symbol_table_info_type_str;
            switch(ELF32_ST_TYPE(symbol_table[index].st_info))
            {
                case STT_NOTYPE     : symbol_table_info_type_str = "Symbol type is unspecified"; break;
                case STT_OBJECT     : symbol_table_info_type_str = "Data object"; break;
                case STT_FUNC       : symbol_table_info_type_str = "Code object "; break;
                case STT_SECTION    : symbol_table_info_type_str = "Symbol associated with a section "; break;
                case STT_FILE		: symbol_table_info_type_str = "Symbol's name is file name "; break;
                case STT_COMMON		: symbol_table_info_type_str = "Symbol is a common data object "; break;
                case STT_TLS		: symbol_table_info_type_str = "Symbol is thread-local data object"; break;
                case STT_NUM		: symbol_table_info_type_str = "Number of defined types.  "; break;
                case STT_LOOS		: symbol_table_info_type_str = "Start of OS-specific "; break;
                case STT_HIOS		: symbol_table_info_type_str = "End of OS-specific "; break;
                case STT_LOPROC		: symbol_table_info_type_str = "Start of processor-specific "; break;
                case STT_HIPROC		: symbol_table_info_type_str = "End of processor-specific "; break;
                default             : symbol_table_info_type_str = "Invalid Type"; break;
            }

            ui->ui_table_symbols->setItem(index,3, new QTableWidgetItem(symbol_table_info_type_str));


            QString symbol_table_other_str;
            switch(ELF32_ST_VISIBILITY(symbol_table[index].st_other))
            {
                case STV_DEFAULT    : symbol_table_other_str = "Default"; break;
                case STV_INTERNAL   : symbol_table_other_str = "Internal"; break;
                case STV_HIDDEN     : symbol_table_other_str = "Hidden"; break;
                case STV_PROTECTED  : symbol_table_other_str = "Protected"; break;
            }

            ui->ui_table_symbols->setItem(index,4, new QTableWidgetItem(QString(symbol_table_other_str)));
            ui->ui_table_symbols->setItem(index,5, new QTableWidgetItem(QString::number(symbol_table[i].st_shndx)));
            ui->ui_table_symbols->setItem(index,6, new QTableWidgetItem("0x"+QString::number(symbol_table[i].st_value,16)));
            ui->ui_table_symbols->setItem(index,7, new QTableWidgetItem(QString::number(symbol_table[i].st_size)));

    */


        }
#endif

     }

     //if (debugInfoAbbrevFound && debugInfoInfoFound && debugInfoStrFound)
     //{
     //   LibParser_DWARF_AbbrevParse(file_descriptor, AbbrevOffset, AbbrevLen);
     //   LibParser_DWARF_InfoParse(file_descriptor, InfoOffset, StrOffset, RODataOffset);
    // }

    fclose(file_descriptor);

    return 0;
}
#endif

bool FileBin_ELF::IsDWARF(void) const
{
    return (debugInfoAbbrevFound && debugInfoInfoFound && debugInfoStrFound);
}


uint32_t FileBin_ELF::GetAbbrevOffset(void) const
{
    return AbbrevOffset;
}

uint32_t FileBin_ELF::GetAbbrevLen(void) const
{
    return AbbrevLen;
}

uint32_t FileBin_ELF::GetInfoOffset(void) const
{
    return InfoOffset;
}

uint32_t FileBin_ELF::GetInfoLen(void) const
{
    return InfoLen;
}

uint32_t FileBin_ELF::GetStrOffset(void) const
{
    return StrOffset;
}
