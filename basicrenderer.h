// Renderer, finally


#include "types.h"
#include "device.h"
#include "vecmath.h"
#include "functions.h"
#include "utils.h"

struct Buffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	uint32_t size;
    VulkanContext* ctx;
	Buffer();
};


struct Texture {
    u32 w;
    u32 h;
    u32 size;
    u32 comps;
    VkImage imageHandle;
    VkImageView view;
    VkDeviceMemory mem;
    VkSampler sampler; // (TODO) allow for more of these
};


struct BasicRenderData {
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout dsLayout;
    VkPipelineLayout plLayout;
    VkPipeline pipeline;
    VkRenderPass rPass;
};

struct BasicVertexData {
    union {
        struct {
            f32 x; f32 y;  f32 z; f32 w;
            f32 u; f32 v;
            f32 nx; f32 ny; f32 nz;
        };
        struct {
            Vector4 coord;
            Vector2 uv;
            Vector3 normal;
        };
    };
    BasicVertexData() {}
};

struct BasicDrawData {
    Buffer matrixUniforms;
    Buffer lightingData;
};

struct BasicMatrices {
    Matrix4 modelViewProjection;
    Matrix4 modelView;
    Matrix3 normalMatrix;
};


struct BasicModel {
    BasicMatrices matrices;
    Vector<BasicVertexData> vData;
    Vector<u32> indices;
    Buffer vertexBuffer;
    Buffer indexBuffer;

    Texture modelTexture;
    VkDescriptorSet descriptorSet;  
    Quaternion rotation;
    f32 scale;
    u32 numPrimitives;
    u32 matrixBufferOffset;
};

struct BasicModelFiles {
    const char* objFile;
    const char* rgbaName;
    const char* normalFile;
};

struct BasicLightData {
    Vector4 position;
    Vector4 lightColor;    
    f32 power;
    f32 shininess;
};

struct Light {
    BasicLightData lightData; 
};

struct BasicFlatScene {
    Vector<BasicModel> models;
    Vector<Light> lights;
    u32 uniformSpace;
  
    
};


struct BasicRenderer {
    
    VulkanContext ctx;
    Buffer stagingBuffer;
    BasicRenderData rData;
    Buffer uniformBasicHostPool;
    Buffer uniformBasicMegaPool;
    Buffer uniformBasicHostLightPool;
    Buffer uniformBasicMegaLightPool;

    VkDescriptorPool basicDescriptorPool;
    VkDescriptorSetLayout basicLayout;
    bool uniformPoolNeedsCopy = true;
    bool lbOK = false;
    bool fenceSet[3];
    bool sceneNeedsUpdate = true;

    
    Buffer MakeBuffer(u32 sizeIn, u32 flagsIn,  VkMemoryPropertyFlagBits wantedProperty);
    Buffer StagingBuffer( u32 sz, void* data);
    Buffer VertexBuffer( u32 sz, void* data);
    void toGPU(void* from, VkDeviceMemory mem, u32 sz );
    Texture MakeTexture(const char* fileName, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, u32 numComps = 4);
    void FillTexture( void* data, Texture* tex, VkFormat format);
    void AllocMemoryImage(u32 sz, VkImage handle, VkMemoryPropertyFlagBits wantedProperty, VkDeviceMemory* mem);

    VkDescriptorSetLayout BasicDescriptorSetLayout(void);
    VkDescriptorPool BasicDescriptorPool(u32 numDescriptors);
    VkDescriptorSet DescriptorSet(VkDescriptorPool* pool, VkDescriptorSetLayout* layout );
    void WriteBasicDescriptorSet(VkDescriptorSet& descriptorSet, BasicModel* model, u32 lights);
    
    void BasicPipelineLayout(BasicRenderData* renderData);
    VkRenderPass BasicRenderPass(VkFormat* swapChainFormat);
    void BasicPipeline(BasicRenderData* renderData);
    VkShaderModule ShaderModule(const char* spirvFileName);
    void MoveUniformFromDMARegion(Buffer &stagingBuffer, Buffer &targetBuffer);
    void MoveVertexBufferFromDMARegion(Buffer &StagingBuffer, Buffer &targetBuffer);


    
    void RefreshFramebuffer(BasicRenderData* rData, VkImageView* imgView, VkFramebuffer* fb );
    void InitBasicRender(u32 numDescriptors);
    BasicModel AddBasicModel(BasicModelFiles fileNames);
    
    void MoveBufferGeneric(Buffer& fromStaging, Buffer& to);
    Buffer IndexBuffer(u32 sz, void* data);
    BasicModel LoadModelObj(const char* f, const char* imageFile);
    u32 CountTrianglesOccurences(const char* f);
    BasicVertexData ConstructVertex(Vector<Vector3>* coords, Vector<Vector3>* normals, Vector<Vector2>* uvcoords, u32 p, u32 t, u32 n  );
    void ParseVertex(const char* s, HashTable* indexHashTable, Vector<u32>* indices, Vector<Vector3>* coords, Vector<Vector3>* normals, Vector<Vector2>* uvcoords,
                     Vector<BasicVertexData>* vertices);


    BasicFlatScene SimpleScene(BasicModel* model, u32 numModels,  BasicLightData* data, u32 numLights);
    void DrawBasicFlatScene(BasicFlatScene* scene);
    Light AddLight(Vector4 pos, Vector4 color, f32 power);
    Light AddLight(const BasicLightData* lightData);
    void Init();

    void SetupUniforms(BasicFlatScene* scene);
    void CopyUniformPool(VkCommandBuffer& cb);

    void UpdateDescriptors(Vector<BasicModel>& models, u32 lights);
    u32  RoundUp(u32);
    bool Runnable();
    void WindowUpdates(void);
    static s32 GetMemoryTypes(u32 typeBits, VkMemoryPropertyFlags propertiesFlags, VkPhysicalDeviceMemoryProperties devProperties);
    VkFormat GetDepthFormat();
    VkWriteDescriptorSet BasicRenderer::DescriptorSetWrite(VkDescriptorSet ds, VkDescriptorType type, u32 bindingNum,
        void* bufferInfo, bool buffer = true);
    
};


