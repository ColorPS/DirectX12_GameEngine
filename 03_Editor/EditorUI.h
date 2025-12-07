#pragma once

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx12.h"
#include "IMGUI/imgui_internal.h"
#include "../02_Engine/d3dx12.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <tchar.h>
#include <DirectXMath.h>

using namespace DirectX;

class EditorApp;
struct RenderItem;

struct ExampleDescriptorHeapAllocator
{
    ID3D12DescriptorHeap* Heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
    D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
    UINT                        HeapHandleIncrement;
    ImVector<int>               FreeIndices;

    void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
    {
        IM_ASSERT(Heap == nullptr && FreeIndices.empty());
        Heap = heap;
        D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
        HeapType = desc.Type;
        HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
        HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
        HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
        FreeIndices.reserve((int)desc.NumDescriptors);
        for (int n = desc.NumDescriptors; n > 0; n--)
            FreeIndices.push_back(n - 1);
    }
    void Destroy()
    {
        Heap = nullptr;
        FreeIndices.clear();
    }
    void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
    {
        IM_ASSERT(FreeIndices.Size > 0);
        int idx = FreeIndices.back();
        FreeIndices.pop_back();
        out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
        out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
    }
    void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
    {
        int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
        int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
        IM_ASSERT(cpu_idx == gpu_idx);
        FreeIndices.push_back(cpu_idx);
    }
};

// 에디터 UI
class EditorUI
{	
	
public:
    ~EditorUI();

public:
    bool Initialize(
        EditorApp* editorApp,
		HWND hwnd,
        ID3D12Device* device,
        ID3D12CommandQueue* commandQueue,
        ID3D12DescriptorHeap* srvHeap,
        DXGI_FORMAT rtvFormat,
        UINT frameCount);	// 초기화
	void Draw(ID3D12GraphicsCommandList* commandList);   // 그리기

private:
    void BeginFrame();  // Draw 시작
    void EndFrame(ID3D12GraphicsCommandList* commandList);  // Draw 끝

    // 도킹 스페이스 그리기
    void DockSpaceDraw();

    // 모든 탭 그리기
    void SceneViewDraw(ID3D12GraphicsCommandList* commandList);
    void GameViewDraw();
    void HierarchyViewDraw();
    void InspectorViewDraw();
    void ProjectViewDraw();

    void SetFocusTab();

public:
    // 프로퍼티
    bool GetIsSceneViewFocused() { return IsSceneViewFocused; }

private:
    bool IsSceneViewFocused;    // Scene뷰에 포커싱이 되어있는지 여부

private:
    EditorApp* mEditorApp = nullptr;

    HWND mHwnd = nullptr;
    ID3D12Device* mDevice = nullptr;
    ID3D12CommandQueue* mCommandQueue = nullptr;
    DXGI_FORMAT mRtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT mFrameCount;

    // DX12 SRV Descriptor Heap 관리용
    ID3D12DescriptorHeap* mSrvHeap = nullptr;

    // 현재 선택한 오브젝트
    RenderItem* mSelectedItem = nullptr;
    bool IsChangeSelectedItem = false;

    // 인스펙터에 사용할 캐시 변수들
    XMFLOAT3 mPosCache;     // Position
    XMFLOAT3 mRotCache;     // Rotation (Euler 각도)
    XMFLOAT3 mScaleCache;   // Scale

};

