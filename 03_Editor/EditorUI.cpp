#include "EditorUI.h"
#include "EditorApp.h"

static ExampleDescriptorHeapAllocator mSrvHeapAlloc;

EditorUI::~EditorUI()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool EditorUI::Initialize(
    EditorApp* editorApp,
    HWND hwnd,
    ID3D12Device* device,
    ID3D12CommandQueue* commandQueue,
    ID3D12DescriptorHeap* srvHeap,
    DXGI_FORMAT rtvFormat,
    UINT frameCount)
{
    mEditorApp = editorApp;
    mHwnd = hwnd;
    mDevice = device;
    mCommandQueue = commandQueue;
    mSrvHeap = srvHeap;
    mRtvFormat = rtvFormat;
    mFrameCount = frameCount;

    // 1) ImGui Context 생성
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    // 2) Win32 백엔드 초기화
    if (!ImGui_ImplWin32_Init(hwnd))
        return false;

    mSrvHeapAlloc.Create(device, mSrvHeap);

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = device;
    init_info.CommandQueue = commandQueue;
    init_info.NumFramesInFlight = frameCount;
    init_info.RTVFormat = rtvFormat;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

    init_info.SrvDescriptorHeap = mSrvHeap;
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return mSrvHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return mSrvHeapAlloc.Free(cpu_handle, gpu_handle); };

    if (!ImGui_ImplDX12_Init(&init_info))
        return false;

    return true;
}

void EditorUI::Draw(ID3D12GraphicsCommandList* commandList)
{
    // 그리기 시작
    BeginFrame();

    // 도킹 스페이스 그리기
    DockSpaceDraw();    

    // 개별 창들 그리기
    SceneViewDraw(commandList); // Scene뷰
    GameViewDraw();             // Game뷰
    HierarchyViewDraw();        // Hierarchy뷰
    InspectorViewDraw();        // Inspector뷰
    ProjectViewDraw();          // Project뷰

    // 그리기 끝
    EndFrame(commandList);
}

void EditorUI::BeginFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void EditorUI::EndFrame(ID3D12GraphicsCommandList* commandList)
{
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

// 도킹 스페이스 그리기
void EditorUI::DockSpaceDraw()
{
    // 처음 실행 시 DockSpace 초기화
    static bool dock_layout_initialized = false;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("DockSpace_Main", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    if (!dock_layout_initialized)
    {
        dock_layout_initialized = true;

        ImGuiID dock_main_id = dockspace_id;

        ImGui::DockBuilderRemoveNode(dock_main_id);
        ImGui::DockBuilderAddNode(dock_main_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dock_main_id, io.DisplaySize);

        // 왼쪽 (1)
        ImGuiID dock_id_L = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.423f, nullptr, &dock_main_id);

        // 왼쪽-위/아래 (2)
        ImGuiID dock_id_LT = ImGui::DockBuilderSplitNode(dock_id_L, ImGuiDir_Up, 0.5f, nullptr, &dock_id_L);
        ImGuiID dock_id_LB = ImGui::DockBuilderSplitNode(dock_id_L, ImGuiDir_Down, 0.5f, nullptr, &dock_id_L);

        // 오른쪽 (1)
        ImGuiID dock_id_R = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.5f, nullptr, &dock_main_id);

        // 아래 (1)  
        ImGuiID dock_id_B = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);

        // 창 도킹
        ImGui::DockBuilderDockWindow("Scene", dock_id_LT);
        ImGui::DockBuilderDockWindow("Game", dock_id_LB);
        ImGui::DockBuilderDockWindow("Hierarchy", dock_main_id);
        ImGui::DockBuilderDockWindow("Inspector", dock_id_R);
        ImGui::DockBuilderDockWindow("Project", dock_id_B);

        ImGui::DockBuilderFinish(dock_main_id);
    }
    ImGui::End();
}

void EditorUI::SceneViewDraw(ID3D12GraphicsCommandList* commandList)
{
    // Scene뷰 시작
    ImGui::Begin("Scene");

    // 포커싱 체크
    SetFocusTab();

    // 현재 뷰에 포커싱이 되어 있는지 체크
    IsSceneViewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    // 버튼 생성
    if (ImGui::Button("Wire Frame")) mEditorApp->SetIsWireFrame(true);
    ImGui::SameLine();
    if (ImGui::Button("Solid")) mEditorApp->SetIsWireFrame(false);

    // GPU Descriptor Heap 바인딩
    ID3D12DescriptorHeap* SrvHeap = mEditorApp->GetSceneSRVHeap();
    commandList->SetDescriptorHeaps(1, &SrvHeap);

    // 화면 비율 유지해서 이미지 크기 설정
    ImVec2 ImageSize = ImGui::GetContentRegionAvail();
    float AspectRatio = 16.0f / 9.0f;
    if (ImageSize.x / ImageSize.y > AspectRatio)
    {
        // 가로가 너무 길면 세로 기준으로 조정
        ImageSize.x = ImageSize.y * AspectRatio;
    }
    else
    {
        // 세로가 너무 길면 가로 기준으로 조정
        ImageSize.y = ImageSize.x / AspectRatio;
    }

    // ImGui에 텍스처 출력
    ImGui::Image((ImTextureID)SrvHeap->GetGPUDescriptorHandleForHeapStart().ptr, ImageSize);

    // Scene뷰 끝
    ImGui::End();
}

void EditorUI::GameViewDraw()
{
    // Game뷰 시작
    ImGui::Begin("Game");

    // 포커싱 체크
    SetFocusTab();

    // Game뷰 화면 보여주기

    // Game뷰 끝
    ImGui::End();
}

void EditorUI::HierarchyViewDraw()
{
    // Hierarchy뷰 시작
    ImGui::Begin("Hierarchy");

    // 포커싱 체크
    SetFocusTab();
    
    // Scene에 있는 오브젝트 종류 보이기
    const auto& Items = mEditorApp->GetAllRItems();
    int index = 0;
    for (const auto& Item : Items)
    {
        std::string label = Item->Name + "##" + std::to_string(index);

        if (ImGui::Selectable(label.c_str(), mSelectedItem == Item.get()))
        {
            mSelectedItem = Item.get();
            IsChangeSelectedItem = true;
        }

        index++;
    }
    
    // Hierarchy뷰 끝
    ImGui::End();
}

void EditorUI::InspectorViewDraw()
{
    // Inspector뷰 시작
    ImGui::Begin("Inspector");

    // 포커싱 체크
    SetFocusTab();
    
    // 현재 선택한 오브젝트 상태 보이기
    if (mSelectedItem)
    {
        ImGui::Text("Name: %s", mSelectedItem->Name.c_str());
        ImGui::Separator();

        // World Matrix 읽어서 Transform 분해
        XMMATRIX World = XMLoadFloat4x4(&mSelectedItem->World);

        XMVECTOR Scale;
        XMVECTOR RotQuat;
        XMVECTOR Pos;

        XMMatrixDecompose(&Scale, &RotQuat, &Pos, World);

        // Vector -> float3로 캐싱
        XMStoreFloat3(&mPosCache, Pos);
        XMStoreFloat3(&mScaleCache, Scale);

        // Euler -> Quaternion 변환
        // 선택한 아이템이 바뀐 경우에만 새로 Rotation 캐시 수정
        if (IsChangeSelectedItem)
        {
            XMFLOAT3 Euler = MathHelper::QuaternionToEuler(RotQuat);
            mRotCache = Euler;
            IsChangeSelectedItem = false;
        }

        // 분류 - Transform
        ImGui::Text("Transform");
        // Position 정보
        if (ImGui::DragFloat3("Position", (float*)&mPosCache, 0.1f))
        {
            mEditorApp->SetRenderItemTransform(mSelectedItem, mPosCache, mRotCache, mScaleCache);
        }

        // Rotation 정보
        if (ImGui::DragFloat3("Rotation", (float*)&mRotCache, 0.1f))
        {
            // 값 순환 처리
            MathHelper::WrapAngle360(mRotCache.x);
            MathHelper::WrapAngle360(mRotCache.y);
            MathHelper::WrapAngle360(mRotCache.z);

            mEditorApp->SetRenderItemTransform(mSelectedItem, mPosCache, mRotCache, mScaleCache);
        }

        // Scale 정보
        if (ImGui::DragFloat3("Scale", (float*)&mScaleCache, 0.1f, 0.0f, 100.0f))
        {
            mEditorApp->SetRenderItemTransform(mSelectedItem, mPosCache, mRotCache, mScaleCache);
        }
    }
    
    // Inspector뷰 끝
    ImGui::End();
}

void EditorUI::ProjectViewDraw()
{
    // Project뷰 시작
    ImGui::Begin("Project");
    
    // 포커싱 체크
    SetFocusTab();

    // 모든 파일 보이기
    
    // Project뷰 끝
    ImGui::End();
}



// 포커싱 체크
void EditorUI::SetFocusTab()
{
    // 좌/우 클릭으로 포커스 설정할 수 있도록 추가
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
    {
        // 이 창을 포커스로 설정
        ImGui::SetWindowFocus();
    }
}
