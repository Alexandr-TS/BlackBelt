#include <cstddef>
#include <string>
#include <vector>
#include <map>

using namespace std;


struct Nucleotide {
    char Symbol; // 2
    size_t Position; // 32
    int ChromosomeNum; // 6
    int GeneNum; // 15
    bool IsMarked; // 1
    char ServiceInfo; // 8
};


struct CompactNucleotide {
    uint32_t Position: 32;
    uint32_t ServiceInfo: 8;
    uint32_t Symbol : 2;
    uint32_t ChromosomeNum : 6;
    uint32_t GeneNum: 15;
    uint32_t IsMarked : 1;
};

string symbols_1 = "ATGC";
map<char, uint32_t> symbols_2 = { {'A', 0}, {'T', 1}, {'G', 2}, {'C', 3} };


bool operator == (const Nucleotide& lhs, const Nucleotide& rhs) {
    return (lhs.Symbol == rhs.Symbol)
        && (lhs.Position == rhs.Position)
        && (lhs.ChromosomeNum == rhs.ChromosomeNum)
        && (lhs.GeneNum == rhs.GeneNum)
        && (lhs.IsMarked == rhs.IsMarked)
        && (lhs.ServiceInfo == rhs.ServiceInfo);
}


CompactNucleotide Compress(const Nucleotide& n) {
    return { 
        static_cast<uint32_t>(n.Position), 
        static_cast<uint32_t>(n.ServiceInfo + 128),
        symbols_2[n.Symbol], 
        static_cast<uint32_t>(n.ChromosomeNum),
        static_cast<uint32_t>(n.GeneNum),
        static_cast<uint32_t>(n.IsMarked)
    };
}

Nucleotide Decompress(const CompactNucleotide& cn) {
    return {
        symbols_1[cn.Symbol],
        static_cast<size_t>(cn.Position),
        static_cast<int>(cn.ChromosomeNum),
        static_cast<int>(cn.GeneNum),
        static_cast<bool>(cn.IsMarked),
        static_cast<char>(cn.ServiceInfo - 128)
    };
}
