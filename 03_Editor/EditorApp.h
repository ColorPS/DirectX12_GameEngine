#pragma once

#include "../02_Engine/d3dApp.h"
#include "../02_Engine/UploadBuffer.h"
#include "../02_Engine/GeometryGenerator.h"
#include "../02_Engine/FrameResource.h"
#include "../02_Engine/Camera.h"

#include "EditorUI.h"

using Microsoft::WRL::ComPtr;           // 
using namespace DirectX;                // 
using namespace DirectX::PackedVector;  // 

extern const int gNumFrameResources;   // 

// 
struct RenderItem
{
    RenderItem() = default;

    std::string Name = "";
    XMFLOAT4X4 World = MathHelper::Identity4x4();

    int NumFramesDirty = gNumFrameResources;

    UINT ObjCBIndex = -1;

    MeshGeometry* Geo = nullptr;

    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
};

class EditorApp : public D3DApp
{
public:
    EditorApp(HINSTANCE hInstance);
    EditorApp(const EditorApp& rhs) = delete;
    EditorApp& operator=(const EditorApp& rhs) = delete;
    ~EditorApp();

public:
    // 초기화
    virtual bool Initialize()override;

    // RenderItem Transform 설정
    void SetRenderItemTransform(RenderItem* Item, const XMFLOAT3& Pos, const XMFLOAT3& RotEuler, const XMFLOAT3& Scale);

private:
    virtual void OnResize()override;                    // 창 크기 조절 시
    virtual void Update(const GameTimer& gt)override;   // 
    virtual void Draw(const GameTimer& gt)override;     // 그리기 관련 처리

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;    // 마우스 클릭 시
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;      // 마우스 클릭 종료 시
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;    // 마우스 이동 시
    void PickItem(int x, int y);

    void OnKeyboardInput(const GameTimer& gt);  // 
    void UpdateObjectCBs(const GameTimer& gt);  // 
    void UpdateMainPassCB(const GameTimer& gt); // 

    void BuildDescriptorHeaps();        // 
    void BuildConstantBufferViews();    // 
    void BuildRootSignature();          // 
    void BuildShadersAndInputLayout();  // 
    void BuildShapeGeometry();          // 
    void BuildPSOs();                   // 
    void BuildFrameResources();         // 
    void BuildRenderItems();            // 
    void SceneHeapsInit();              // Scene Heap 설정

    void DrawSceneView();   // Scene뷰 생성
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);   // 

public:
    // 프로퍼티
    ID3D12DescriptorHeap* GetSceneSRVHeap() { return mSceneSRVHeap.Get(); }
    std::vector<std::unique_ptr<RenderItem>>& GetAllRItems() {return mAllRitems;}

    void SetIsWireFrame(bool IsWireFrame) { mIsWireframe = IsWireFrame; }

private:
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;    //
    FrameResource* mCurrFrameResource = nullptr;                    // 
    int mCurrFrameResourceIndex = 0;                                // 

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;   // 
    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;        // 

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;  //

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries; //
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;                 // 
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;         // 

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout; // 

    std::vector<std::unique_ptr<RenderItem>> mAllRitems;    // 

    std::vector<RenderItem*> mOpaqueRitems; // 

    PassConstants mMainPassCB;  //

    UINT mPassCbvOffset = 0;    // 

    bool mIsWireframe = false;  // WireFrame모드 여부

    // 마우스 클릭 했는지 여부
    bool IsMouseDown = false;

    float mTheta = 1.5f * XM_PI;
    float mPhi = 0.2f * XM_PI;
    float mRadius = 15.0f;

    POINT mLastMousePos;

    EditorUI mEditorUI;

    // 카메라
    Camera mSceneCamera;

    // Scene뷰 관련 변수
    ComPtr<ID3D12Resource> mSceneTexture;       // 카메라 시점을 담을 텍스처
    ComPtr<ID3D12DescriptorHeap> mSceneSRVHeap; // ImGui용 SRV Heap
    ComPtr<ID3D12DescriptorHeap> mRtvHeap;      // ImGui용 RTV Heap
    D3D12_CPU_DESCRIPTOR_HANDLE mSceneRTV;      // mSceneTexture 용 RTV
};