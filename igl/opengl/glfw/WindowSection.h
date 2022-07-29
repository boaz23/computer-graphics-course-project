#include <vector>
#include <set>
#include <igl/opengl/glfw/SectionLayer.h>
#include <igl/opengl/glfw/DrawInfo.h>

enum drawFlags {
    toClear = 1,
    is2D = 2,
    inAction = 4,
    scissorTest = 8,
    depthTest = 16,
    stencilTest = 32,
    blend = 64,
    blackClear = 128,
    clearDepth = 256,
    backdraw = 512,
    clearStencil = 1024,
    passStencil = 2048,
    inAction2 = 4096,
    none = 8192,
    scaleAbit = 16384,
    stencil2 = 32768,
    onPicking = 65536
};

class WindowSection {
public:
    WindowSection(int x, int y, int z, int w, 
        int buffIndex, int property_id, int index,
        bool createStencilLayer,
        bool createScissorsLayer,
        bool clearBuffer,
        bool autoAddToSection=true,
        bool allowRotation=true) {
        viewportDims = Eigen::Vector4i(x, y, z, w);
        sectionCameraIndex = -1;
        active = false;
        this->autoAddToSection = autoAddToSection;
        this->allowRotation = allowRotation;
        sceneLayerIndex = AddSectionLayer();
        AddDraw(sceneLayerIndex, buffIndex,
            (clearBuffer & toClear) | depthTest | clearDepth | stencilTest |
            passStencil | clearStencil | blend,
            property_id
        );
        scissorsTestLayerIndex = -1;
        stencilTestLayerIndex = -1;
        property_id <<= 1;
        if (createScissorsLayer) {
            scissorsTestLayerIndex = AddSectionLayer();
            AddDraw(scissorsTestLayerIndex, buffIndex,
                depthTest |
                blend | inAction2 | scissorTest,
                property_id);
            property_id <<= 1;
        }
        if (createStencilLayer) {
            stencilTestLayerIndex = AddSectionLayer();
            AddDraw(stencilTestLayerIndex, buffIndex,
                onPicking | stencilTest |
                scaleAbit | inAction | stencil2 | depthTest,
                property_id);
        }
    };

    inline int AddDraw(int layerIndex, int buffIndx, unsigned int flags, int property_id) { return sectionLayers[layerIndex]->AddDraw(buffIndx, flags, property_id); };

    inline void SetDrawFlag(int layerIndex, int infoIndx, unsigned int flag) { sectionLayers[infoIndx]->SetDrawFlag(infoIndx, flag); }

    inline void ClearDrawFlag(int layerIndex, int infoIndx, unsigned int flag) { sectionLayers[infoIndx]->ClearDrawFlag(infoIndx, flag); }

    inline DrawInfo& GetDrawInfo(int layerIndex, int index) { return sectionLayers[layerIndex]->GetDrawInfo(index); }

    inline void SetCamera(int cameraIndex) { sectionCameraIndex = cameraIndex; }
    inline int GetCamera() const { return sectionCameraIndex; }

    inline Eigen::Vector4i GetViewportSize() const { return viewportDims; }
    inline void SetViewportSize(Eigen::Vector4i newSize) { viewportDims = newSize; }

    inline int GetSceneLayerIndex() { return sceneLayerIndex; }
    inline int GetScissorTestLayerIndex() { return scissorsTestLayerIndex; }
    inline int GetStencilTestLayerIndex() { return stencilTestLayerIndex; }

    inline int AddSectionLayer() { 
        sectionLayers.push_back(new SectionLayer());
        return (int)sectionLayers.size() - 1;
    }
    inline std::vector<SectionLayer*> GetLayers() { return sectionLayers; }
    ~WindowSection() {
        for (auto& layer : sectionLayers) {
            delete layer;
        }
    }

    inline void Deactivate() { active = false; }
    inline void Activate() { active = true; }
    inline bool isActive() { return active; }

    inline bool IsAutoAddSection() { return autoAddToSection; }
    inline bool IsRotationAllowed() { return allowRotation; }
private:
    int sectionCameraIndex;
    std::vector<SectionLayer*> sectionLayers;
    Eigen::Vector4i viewportDims;
    int sceneLayerIndex;
    int scissorsTestLayerIndex;
    int stencilTestLayerIndex;
    bool active;
    bool autoAddToSection;
    bool allowRotation;
};