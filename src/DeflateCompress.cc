#include "Deflate.h"
#include <unordered_map>
#include <list>
using std::unordered_map;
using std::list;

const int WINDOW_WIDTH = 3;
const int MAX_DIST = 32768;
const int MAX_LENGTH = 258;

struct SubChainPointer
{
    int idx = 0; //used in hashmap and output

    //only used in the output buffer
    int length = 0; 
    bool duplicated = false;
};

vector<uint8_t> Deflate::compress(vector<uint8_t> &data)//BitStream &in)
{
    unordered_map<uint32_t, list<SubChainPointer>> hashmap;
    vector<SubChainPointer> output; //either points to unaltered chains in the original data or duplicated sequences that references previously appearing chains
    auto hashFunc = [&](int idx){
        return (uint32_t(data[idx])<<16)|(uint32_t(data[idx+1])<<8)|data[idx+2];
    };

    for(int i = 0; i < data.size()-WINDOW_WIDTH; ) //for every byte
    {
        uint32_t hash = hashFunc(i); //we hash the window starting from this byte
        bool duplicatedFound = false;

        if(hashmap.find(hash)!=hashmap.end())//found in hashmap
        {
            SubChainPointer maxChain{-1,-1,false};
            for(auto it = hashmap[hash].begin(); it != hashmap[hash].end(); it++)
            {
                int idx = it->idx;
                if(i-idx > MAX_DIST) break;  
                int length = 0;
                for(int j = i; data[idx] == data[j]; j++,idx++,length++);
                assert(length >= WINDOW_WIDTH);
                if(length > maxChain.length)
                {
                    maxChain.duplicated = true;
                    maxChain.length = length;
                    maxChain.idx = it->idx;
                }
            }
            assert(maxChain.duplicated);
            if(maxChain.duplicated)
            {
                output.push_back(maxChain);
                hashmap[hash].push_front(maxChain);
                i+=maxChain.length;
                duplicatedFound = true;
            }
            else //we didn't find a duplicated chain close enough
            {
                ;//nothing to do: the duplicatedFound boolean is unaltered(remains false -> triggers next if)
            }
        }
        if(!duplicatedFound)
        {
            SubChainPointer chainPtr{i,1,false};//we create the chain ptr
            int outSize = output.size();
            if(outSize > 0)
            {
                if(output[outSize-1].duplicated)
                {
                    output.push_back(chainPtr); 
                }
                else
                {
                    output[outSize-1].length++;
                }
            }
            else
            {
                output.push_back(chainPtr); 
            }
            
            hashmap[hash] = list<SubChainPointer>();
            hashmap[hash].push_front(chainPtr);

            i++;
        }
    }

    //Now: Generate blocks with huffman codes
    loadFixedHuffmanTrees();
    BitStream out;
    out.write(1,1);//final block
    out.write(1,2); //fixed huffman
    int currentIdx = 0;
    for(SubChainPointer ptr : output)
    {
        if(!ptr.duplicated)
        {
            for(int i = ptr.idx; i < ptr.idx+ptr.length; i++)
            {
                literalTree.write(out,data[i]);
            }
            currentIdx+=ptr.length;
        }
        else
        {
            SelectedParse parsed = tableEncode(out,lengthTable,ptr.length);
            literalTree.write(out,parsed.code);
            out.write(parsed.extra,parsed.extrabit);

            parsed = tableEncode(out,distTable,currentIdx-ptr.idx);
            distanceTree.write(out,parsed.code);
            out.write(parsed.extra,parsed.extrabit);

            currentIdx+=ptr.length;
        }
    }
    literalTree.write(out,256);//end of block marker

    return out.getData();
}