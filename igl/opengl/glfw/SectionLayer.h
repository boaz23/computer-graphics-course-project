#pragma once

class SectionLayer {
public:
    SectionLayer() {};
    inline int AddDraw(int buffIndx, unsigned int flags, int property_id){
        drawInfos.emplace_back(new DrawInfo(buffIndx, flags, property_id));
        return (int)drawInfos.size() - 1;
    };
    inline void SetDrawFlag(int infoIndx, unsigned int flag) { drawInfos[infoIndx]->SetFlags(flag); }

    inline void ClearDrawFlag(int infoIndx, unsigned int flag) { drawInfos[infoIndx]->ClearFlags(flag); }

    inline DrawInfo& GetDrawInfo(int index) { return *drawInfos[index]; }

    inline std::vector<DrawInfo*> getInfos() { return drawInfos; }

    ~SectionLayer() {
        for (auto& info : drawInfos) {
            delete info;
        }
    }
private:
	std::vector<DrawInfo*> drawInfos;
};