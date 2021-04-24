#include "Common.h"

#include "BitStream.h"
#include "Deflate.h"
#include "HuffmanTree.h"
#include "Base64.h"
#include "tests/Tests.h"

int main()
{
    cout << "The PNG Project" << endl;
    //mainTests();

    vector<uint8_t> data;
    
    string compressedMsg = "VVBBbgMhDPyKb7lEeUTSjzjgVEQYEzCrqq/pX/qxjrvtYS2wkTUzHnO19rQ1zrRZqd9ftBq5fLjQawk9hfTEvQ9BgymZ4jmnDOrgRAJrgDAdPbVGfRggUd4HqwrlOP/EAkSWR2WXc4i3X313aZk6zxixD05SmR7WkoMCVF6QFC2Ykmx1Ui6T7tYuCHoDLtS2aKqsewUMe3Tx4shrd5tDaRDD/2fYUnboHfe60HX/jxA/1Pge/svVasGl2wmL0za4qDSnUXJJq8rB3R4/";
    //data = b64_decode(compressedMsg);
    

    
    string clearMsg = "Bonjour, bonjour, bonjour, j'espere que vous allez tres bien !!!!!!! Bonjour que vous aller aller dfvdfvrbtrszzzzcddbgd hihihihihhahahaha";
    clearMsg = "#include \"Deflate.h\"\r\n#include <array>\r\nusing std::array;\r\n#include \"HuffmanTree.h\"\r\n\r\nDeflate::Deflate()\r\n{\r\n\r\n}\r\n\r\nvoid Deflate::processUncompressedBlock(BitStream &in)\r\n{\r\n    in.skipToFullByte(BitStream::ReadCursor);\r\n    uint16_t len = in.read16(16);\r\n    uint16_t nlen = ~in.read16(16);\r\n\r\n    if(len != nlen) throw \"[BitStream] Incoherent header for Uncompressed block: len and nlen don\'t match\";\r\n    for(; len > 0; --len)\r\n    {\r\n        out.write(in.read(8),8);\r\n    }\r\n}\r\n\r\nBitStream& Deflate::uncompress(BitStream &in)\r\n{\r\n    while(!finalBlock)\r\n    {\r\n        processBlock(in);\r\n    }\r\n    return out;\r\n}\r\n\r\nvoid Deflate::loadDynamicHuffmanTrees(BitStream& in)\r\n{\r\n    unsigned int HLIT = in.read(5)+257;\r\n    unsigned int HDIST = in.read(5)+1;\r\n    unsigned int HCLEN = in.read(4)+4;\r\n\r\n    vector<int> codeLengthAlphabet = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};\r\n    vector<int> codeLengthCodes = {};\r\n    for(int i = 0; i < codeLengthAlphabet.size(); i++)\r\n        codeLengthCodes.push_back(0);\r\n    for(unsigned int i = 0;i<HCLEN;i++)\r\n    {\r\n        \/\/codeLengthCodes.push_back(in.read(3));\r\n        codeLengthCodes[codeLengthAlphabet[i]] = in.read(3);\r\n    }\r\n    HuffmanTree<int> predecodeTree;\r\n    \/\/predecodeTree.loadFromCodeLength(codeLengthAlphabet,true,codeLengthCodes,7);\r\n    predecodeTree.loadFromCodeLength(codeLengthCodes);\r\n\r\n    vector<int> globalCodeLengths;\r\n    int lastLength = 0;\r\n\r\n    for(unsigned int totalCodeLength = 0; totalCodeLength < HDIST+HLIT;)\r\n    {\r\n        int v = predecodeTree.readNext(in);\r\n        if(v < 16)\r\n        {\r\n            lastLength = v;\r\n            totalCodeLength++;\r\n            globalCodeLengths.push_back(v);\r\n        }\r\n        else if(v == 16)\r\n        {\r\n            int repeatLast = in.read(2)+3;\r\n            totalCodeLength += repeatLast;\r\n            for(;repeatLast>0;repeatLast--)\r\n                globalCodeLengths.push_back(lastLength);\r\n        }\r\n        else\r\n        {\r\n            int repeatZero = 0;\r\n            if(v==17)\r\n                repeatZero = in.read(3)+3;\r\n            else\r\n                repeatZero = in.read(7)+11;\r\n            lastLength = 0;\r\n            totalCodeLength += repeatZero;\r\n            for(;repeatZero>0;repeatZero--)\r\n                globalCodeLengths.push_back(0);\r\n        }\r\n    }\r\n    if(globalCodeLengths.size() != HLIT+HDIST) throw \"dynamic huffman error: hdit and hlit don\'t match the number of code length read\";\r\n\r\n    vector<int> distLengths;\r\n    for(unsigned int i = HLIT; i < globalCodeLengths.size(); i++)\r\n        distLengths.push_back(globalCodeLengths[i]);\r\n    globalCodeLengths.resize(HLIT);\/\/becomes literal\/length lengths\r\n\r\n    literalTree.loadFromCodeLength(globalCodeLengths);\r\n    distanceTree.loadFromCodeLength(distLengths);\r\n}\r\n\r\nvoid Deflate::loadFixedHuffmanTrees()\r\n{\r\n    vector<int> literalCodeLength;\r\n    for(int i = 0; i <= 143; i++)\r\n        literalCodeLength.push_back(8);\r\n    for(int i = 144; i <= 255; i++)\r\n        literalCodeLength.push_back(9);\r\n    for(int i = 256; i <= 279; i++)\r\n        literalCodeLength.push_back(7);\r\n    for(int i = 280; i <= 287; i++)\r\n        literalCodeLength.push_back(8);\r\n\r\n    vector<int> distCodeLength;\r\n    for(int i = 0; i <= 31; i++)\r\n        distCodeLength.push_back(5);\r\n\r\n    literalTree.loadFromCodeLength(literalCodeLength,9);\r\n    distanceTree.loadFromCodeLength(distCodeLength,5);\r\n}\r\nDeflate::SelectedParse Deflate::tableParse(uint16_t val, vector<uint16_t> const &thresholds, vector<uint16_t> const &extrabits,vector<uint16_t> const &minValues)\r\n{\r\n    int i = thresholds.size()-1;\r\n    for(; i>=0; i--)\r\n    {\r\n        if(val >= thresholds[i])\r\n            break;\r\n    }\r\n\r\n    \/\/uint16_t diff = val-threshold[i];\r\n    \/\/uint16_t step = 1<<extrabit[i];\r\n    return SelectedParse{  \r\n                    threshold: thresholds[i],\r\n                    extrabit: extrabits[i],\r\n                    minValue: minValues[i],\r\n                    diff: val-thresholds[i],\r\n                    step: 1<<extrabits[i]};\r\n}\r\nuint16_t Deflate::tableDecode(BitStream & in, ParseTables const & table, uint16_t val)\/\/, vector<uint16_t> const &threshold, vector<uint16_t> const &extrabit,vector<uint16_t> const &minValue)\r\n{\r\n    SelectedParse parsed = tableParse(val, table.threshold, table.extrabit, table.minValue);\r\n\r\n    uint16_t base = parsed.diff*parsed.step+parsed.minValue;\r\n    return base+in.read(parsed.extrabit);\r\n}\r\n\r\nDeflate::SelectedParse Deflate::tableEncode(BitStream & out, ParseTables const & table, uint16_t val)\r\n{\r\n    SelectedParse parsed = tableParse(val, table.minValue, table.extrabit, table.threshold);\/\/careful: we need to reverse the threshold and minValue(TODO: rename those fields)\r\n\r\n    int level = parsed.diff \/ parsed.step;\r\n    int extra = parsed.diff % parsed.step;\r\n    int code = parsed.minValue + level;\r\n\r\n    parsed.extra = extra;\r\n    parsed.code = code;\r\n    \/\/distanceTree.write(out,code);\r\n    \/\/literalTree.write(out,code);\r\n    \/\/out.write(extra,parsed.extrabit);\r\n    return parsed;\r\n}\r\n\r\nuint16_t Deflate::computeDistance(BitStream & in, uint16_t dist)\r\n{\r\n    return tableDecode(in,distTable,dist);\r\n}\r\n\r\nuint16_t Deflate::computeLength(BitStream & in, uint16_t len)\r\n{\r\n    return tableDecode(in,lengthTable,len);\r\n}\r\n\r\nvoid Deflate::processDuplicatedSequence(BitStream & in, uint16_t len)\r\n{\r\n    if(len == 256) return;\r\n    uint16_t length = computeLength(in,len); \/\/tableDecode(in,lengthTable,len);\/\/\r\n    uint16_t distance = computeDistance(in,distanceTree.readNext(in)); \/\/tableDecode(in,distTable,distanceTree.readNext(in));\/\/\r\n\r\n    vector<uint8_t> const & buffer(out.getBufferRef());\r\n    int initialEndOfBuffer = buffer.size();\r\n    int startIndex = initialEndOfBuffer-distance;\r\n    if(startIndex < 0) throw \"Deflate Error: A duplicated sequence can\'t reference data before the beginning of the output stream\";\r\n\r\n    int currentIndex = startIndex;\r\n    for(;length > 0; length--)\r\n    {\r\n        out.write(buffer[currentIndex],8);\r\n        currentIndex++;\r\n        if(currentIndex>=initialEndOfBuffer)\r\n            currentIndex = startIndex;\r\n    }\r\n}\r\n\r\nvoid Deflate::processCompressedBlock(BitStream & in)\r\n{\r\n    uint16_t litLength;\r\n    do\r\n    {\r\n        litLength = literalTree.readNext(in);\r\n        if(litLength < 256)\r\n            out.write(litLength,8);\r\n        else\r\n            processDuplicatedSequence(in,litLength);\r\n    }while(litLength != 256);\r\n}\r\n\r\nvoid Deflate::processBlock(BitStream &in)\r\n{\r\n    finalBlock = in.read(1);\r\n    int blockType = in.read(2);\r\n    switch(blockType)\r\n    {\r\n    case 0: \/\/Uncompressed block\r\n        processUncompressedBlock(in);\r\n        break;\r\n    case 1:\r\n        loadFixedHuffmanTrees();\r\n    case 2:\r\n        if(blockType == 2)\/\/dynamic huffman\r\n        {\r\n            loadDynamicHuffmanTrees(in);\r\n        }\r\n        processCompressedBlock(in);\r\n        break;\r\n    case 3:\r\n        throw \"Deflate Error: use of reserved type block - Aborting\";\r\n        return;\r\n    default:\r\n        cout << \"Deflate\/DataStream error: impossible value found: \"<<blockType<<endl;\r\n        return;\r\n    }\r\n}\r\n";
    for(char c : clearMsg)
        data.push_back(c);
    
    int originalSize = data.size();

    try
    {
        //BitStream bs(data);
        Deflate deflator;
        
        /*BitStream& os(deflator.uncompress(bs));

        vector<uint8_t> result = os.getData();
        for(uint8_t r : result)
            cout << char(r);*/
        

        data = deflator.compress(data);
        int compressedSize = data.size();
        BitStream bs(data);
        //deflator.uncompress(bs);
        BitStream& os(deflator.uncompress(bs));

        vector<uint8_t> result = os.getData();
        for(uint8_t r : result)
            cout << char(r);

        cout << endl<< "Compressed: " << b64_encode(data) << endl;

        cout << "Compression ratio:"<<endl<<"Original size: "<<originalSize<<endl<<"New size: "<<compressedSize<<endl;
        cout << "Ratio: "<< (double(compressedSize)/originalSize)*100.0<<" %" <<endl;
    }
    catch(const char * msg)
    {
        cerr << "Exception: "<<msg<<endl;
    }


    return 0;
}
