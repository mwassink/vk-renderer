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
            f32 nx; f32 ny; f32 nz;
            f32 u; f32 v;
        };
        struct {
            Vector4 coord;
            Vector3 normal;
            Vector2 uv;
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
};


struct BasicModel {
    BasicMatrices matrices;
    Vector<BasicVertexData> vData;
    Vector<u32> indices;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    Buffer matrixBuffer;
    Texture modelTexture;
    Quaternion rotation;
    f32 scale;
    u32 numPrimitives;
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
    Buffer uniformBuffer;
};

struct BasicFlatScene {
    Vector<BasicModel> models;
    Vector<Light> lights;
    
};


struct Renderer {
    
    VulkanContext ctx;
    Buffer stagingBuffer;
    BasicRenderData rData;
    
    
    Buffer MakeBuffer(u32 sizeIn, u32 flagsIn,  VkMemoryPropertyFlagBits wantedProperty);
    
    Buffer UniformBuffer( u32 sz, void* data);
    Buffer StagingBuffer( u32 sz, void* data);
    Buffer VertexBuffer( u32 sz, void* data);
    void toGPU(void* from, VkDeviceMemory mem, u32 sz );
    Texture RGBATexture(const char* fileName);
    void FillTexture(u32 sz, void* data, Texture* tex);
    void AllocMemoryImage(u32 sz, VkImage handle, VkMemoryPropertyFlagBits wantedProperty, VkDeviceMemory* mem);

    void BasicRenderPass(void);

    VkDescriptorSetLayout BasicDescriptorSetLayout(void);
    VkDescriptorPool BasicDescriptorPool(void);
    VkDescriptorSet BasicDescriptorSetAllocation(VkDescriptorPool* pool, VkDescriptorSetLayout* layout );
    void WriteBasicDescriptorSet(BasicDrawData* renderData, VkDescriptorSet& descriptorSet, BasicModel* model);
    
    void BasicPipelineLayout(BasicRenderData* renderData);
    VkRenderPass BasicRenderPass(VkFormat* swapChainFormat);
    VkPipeline BasicPipeline(BasicRenderData* renderData);
    VkShaderModule ShaderModule(const char* spirvFileName);
    void MoveUniformFromDMARegion(Buffer &stagingBuffer, Buffer &targetBuffer);
    void MoveVertexBufferFromDMARegion(Buffer &StagingBuffer, Buffer &targetBuffer);
    void BasicRenderModel(BasicModel* model);
    void DrawBasic(BasicRenderData* renderData, VkImageView* imgView, VkFramebuffer* currentFB, VkCommandBuffer cb, VkImage img, BasicModel* model);
    void RefreshFramebuffer(BasicRenderData* rData, VkImageView* imgView, VkFramebuffer* fb );
    BasicRenderData InitBasicRender(void);
    BasicModel AddBasicModel(BasicModelFiles fileNames);
    void UpdateModel(BasicModel* model);
    void LightPassInternal(Vector<BasicModel>& model, Light* light, BasicRenderData* rData,
                   PerFrameData* frameData, Image* img   );
    
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
    

    
   
    
};


