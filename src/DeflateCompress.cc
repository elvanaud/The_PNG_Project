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
    int dist = 0;
};

vector<uint8_t> Deflate::compress(vector<uint8_t> &data)//BitStream &in)
{
    unordered_map<uint32_t, list<SubChainPointer>> hashmap;
    vector<SubChainPointer> output; //either points to unaltered chains in the original data or duplicated sequences that references previously appearing chains
    auto hashFunc = [&](int idx){
        return (uint32_t(data[idx])<<16)|(uint32_t(data[idx+1])<<8)|data[idx+2];
    };

    //array<int,286> litFreq = {};
    vector<int> litFreq(286,0);
    vector<int> distFreq(30,0);
    //array<int,30>  distFreq = {};

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
            //assert(maxChain.duplicated);//TODO: this wasn't commented and yet it worked
            if(maxChain.duplicated)
            {
                maxChain.dist = i-maxChain.idx;
                SelectedParse parsed = tableEncode(out,lengthTable,maxChain.length);
                litFreq[parsed.code]++;
                parsed = tableEncode(out,distTable,maxChain.dist);
                distFreq[parsed.code]++;

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
            litFreq[data[i]]++;
            
            hashmap[hash] = list<SubChainPointer>();
            hashmap[hash].push_front(chainPtr);

            i++;
        }
    }
    assert(litFreq[256] == 0);
    litFreq[256] = 1;

    generateDynamicTrees(litFreq,literalTree);//distFreq);
    generateDynamicTrees(distFreq,distanceTree);

    litFreq.insert(litFreq.end(),distFreq.begin(),distFreq.end());
    generateDynamicHeader(litFreq); 

    //Now: Generate blocks with huffman codes
    //loadFixedHuffmanTrees();
    BitStream out;
    out.write(1,1);//final block
    out.write(2,2); 
    //int currentIdx = 0;
    for(SubChainPointer ptr : output)
    {
        if(!ptr.duplicated)
        {
            for(int i = ptr.idx; i < ptr.idx+ptr.length; i++)
            {
                literalTree.write(out,data[i]);
            }
            //currentIdx+=ptr.length;
        }
        else
        {
            SelectedParse parsed = tableEncode(out,lengthTable,ptr.length);
            literalTree.write(out,parsed.code);
            out.write(parsed.extra,parsed.extrabit);

            parsed = tableEncode(out,distTable,ptr.dist);//currentIdx-ptr.idx);
            distanceTree.write(out,parsed.code);
            out.write(parsed.extra,parsed.extrabit);

            //currentIdx+=ptr.length;
        }
    }
    literalTree.write(out,256);//end of block marker

    return out.getData();
}

struct Node{
        int freq;
        int childA=-1,childB=-1;
    };

void countOccs(Node r, int prefixLength, vector<int> & lengthOccurences, vector<Node> & tree)
{
    if(r.childA == -1 && r.childB == -1)
    {
        while(prefixLength >= lengthOccurences.size())
        {
            lengthOccurences.push_back(0);
        }
        lengthOccurences[prefixLength]++;
    }
        

    if(r.childA != -1) countOccs(tree[r.childA],prefixLength+1,lengthOccurences,tree);
    if(r.childB != -1) countOccs(tree[r.childB],prefixLength+1,lengthOccurences,tree);
    
}

void Deflate::generateDynamicTrees(vector<int> & litFreq_p, HuffmanTree<uint16_t> & binTree)//, vector<int> & distFreq_p)
{
    vector<int> huffLitFreq(litFreq_p);
    //vector<int> huffDistFreq(distFreq_p);

    huffLitFreq.erase(std::remove(huffLitFreq.begin(), huffLitFreq.end(), 0), huffLitFreq.end());
    //huffDistFreq.erase(std::remove(huffDistFreq.begin(), huffDistFreq.end(), 0), huffDistFreq.end());

    std::sort(huffLitFreq.begin(),huffLitFreq.end());
    //std::sort(huffDistFreq.begin(),huffDistFreq.end());

    //count the number of codes for each length:
    

    vector<Node> tree;
    vector<Node> huffLit;
    //vector<Node> huffDist;

    for(int f : huffLitFreq)
        huffLit.push_back(Node{f});

    while(huffLit.size()>= 2)
    {
        Node n{huffLit[0].freq+huffLit[1].freq};
        tree.push_back(huffLit[0]);
        tree.push_back(huffLit[1]);
        int s = tree.size();
        n.childA = s-2;
        n.childB = s-1;

        int i;
        for(i = 2;  i < huffLit.size() && huffLit[i].freq < n.freq; i++)
            huffLit[i-2] = huffLit[i];
        huffLit[i-2] = n;
        for(;i < huffLit.size();i++)
            huffLit[i-1] = huffLit[i];
        huffLit.resize(huffLit.size()-1);
    }
    tree.push_back(huffLit[0]);
    //count occurences of each length:
    vector<int> lengthOccurences;
    
    countOccs(tree[tree.size()-1],1,lengthOccurences,tree);

    //associate short code lengths to most frequent symbols
    for(int length = 1;length < lengthOccurences.size(); length++)
    {
        int nbOcc = lengthOccurences[length];
        if(nbOcc == 0) continue;
        vector<int> highFreqSymbols(nbOcc,0);

        for(int idx = 0; idx < litFreq_p.size(); idx++)
        {
            if(litFreq_p[idx] <= 0) continue; //already found
            int i;
            for(i=nbOcc-1; i>=1 && litFreq_p[idx]>litFreq_p[highFreqSymbols[i]];i--) highFreqSymbols[i] = highFreqSymbols[i-1];
            highFreqSymbols[i] = idx;
        }

        for(int symIdx : highFreqSymbols)
        {
            litFreq_p[symIdx] = -length;
        }
    } 

    for(int i = 0; i < litFreq_p.size(); i++)
        litFreq_p[i] *= -1;

    binTree.loadFromCodeLength(litFreq_p);
}

void Deflate::generateDynamicHeader(vector<int> & codeLengths)
{
    struct AlphaHeader{
        int code;
        int bits;
    };

    vector<AlphaHeader> rawHeader;

    int previousLength = 0;
    int matchingOccs = 0;
    for(int length : codeLengths)
    {
        if(length == previousLength)
        {
            matchingOccs++;
            if(previousLength != 0 && matchingOccs == 6)
            {
                rawHeader.push_back(AlphaHeader{16,matchingOccs-3});
                matchingOccs = 0;
            }
            if(previousLength == 0 && matchingOccs == 138)
            {
                rawHeader.push_back(AlphaHeader{18,matchingOccs-11});
                matchingOccs = 0;
            }
        }
        else
        {
            if(matchingOccs >= 3)
            {
                if(previousLength != 0 && matchingOccs <= 6)
                {
                    rawHeader.push_back(AlphaHeader{16,matchingOccs-3});
                }
                else if(previousLength == 0 && matchingOccs <= 10)
                {
                    rawHeader.push_back(AlphaHeader{17,matchingOccs-3});
                }
                else if(previousLength == 0 && matchingOccs <= 138)
                {
                    rawHeader.push_back(AlphaHeader{18,matchingOccs-11});
                }
            }
            else //if(matchingOccs > 0)//not enough occurences to mark duplicate
            {
                for(;matchingOccs>0;matchingOccs--)rawHeader.push_back(AlphaHeader{0});
            }

            if(length != 0)
            {
                rawHeader.push_back(AlphaHeader{length});
                matchingOccs = 0;
                previousLength = length;
            }
            else
            {
                matchingOccs = 1;
                previousLength = 0;
            }
            
        }
    }

    cout<<"h";
}