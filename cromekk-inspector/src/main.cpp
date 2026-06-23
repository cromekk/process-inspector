#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <string>
#include "inspector.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

DWORD targetPid = 0;
HANDLE targetHandle = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam );

/* window_procedure: Routes mandatory OS messages and forwards mouse/keyboard inputs directly into the ImGui interface. */
LRESULT CALLBACK window_procedure( HWND window,UINT message,WPARAM w_param,LPARAM l_param )
{
	if ( ImGui_ImplWin32_WndProcHandler( window,message,w_param,l_param ) ) {
		return 0L;
	}

	if ( message == WM_DESTROY )
	{
		PostQuitMessage( 0 );
		return 0L;
	}

	return DefWindowProc( window,message,w_param,l_param );
}

/* WinMain: Configures sharp DPI scaling, builds the borderless window, connects the D3D11 renderer, and runs the main loop. */
INT APIENTRY WinMain( HINSTANCE instance,HINSTANCE,PSTR,INT cmd_show ) {
	SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

	WNDCLASSEXW wc{};
	wc.cbSize = sizeof( WNDCLASSEXW );
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = instance;
	wc.lpszClassName = L"cromekk-base Overlay Class";

	RegisterClassExW( &wc );

	const HWND window = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_LAYERED,
		wc.lpszClassName,
		L"cromekk-base",
		WS_POPUP,
		0,
		0,
		1920,
		1080,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	SetLayeredWindowAttributes( window,RGB( 0,0,0 ),BYTE( 255 ),LWA_ALPHA );

	{
		RECT client_area{};
		GetClientRect( window,&client_area );

		RECT window_area{};
		GetWindowRect( window,&window_area );

		POINT diff{};
		ClientToScreen( window,&diff );

		const MARGINS margins{
			window_area.left + ( diff.x - window_area.left ),
			window_area.top + ( diff.y - window_area.top ),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea( window,&margins );
	}

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2U;
	sd.OutputWindow = window;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	constexpr D3D_FEATURE_LEVEL levels[2]{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	ID3D11Device* device{ nullptr };
	ID3D11DeviceContext* device_context{ nullptr };
	IDXGISwapChain* swap_chain{ nullptr };
	ID3D11RenderTargetView* render_target_view{ nullptr };
	D3D_FEATURE_LEVEL level{};

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		levels,
		2U,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&level,
		&device_context
	);

	ID3D11Texture2D* back_buffer{ nullptr };
	swap_chain->GetBuffer( 0U,IID_PPV_ARGS( &back_buffer ) );

	if ( back_buffer ) {
		device->CreateRenderTargetView( back_buffer,nullptr,&render_target_view );
		back_buffer->Release();
	}
	else {
		return 1;
	}

	ShowWindow( window,cmd_show );
	UpdateWindow( window );

	ImGui::CreateContext();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 10.0f;
	style.FrameRounding = 6.0f;
	style.CellPadding = ImVec2( 10.0f,8.0f );
	style.ItemSpacing = ImVec2( 12.0f,10.0f );
	style.ScrollbarRounding = 12.0f;
	style.TabRounding = 6.0f;
	style.WindowPadding = ImVec2( 15.0f,15.0f );

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_WindowBg] = ImVec4( 0.06f,0.07f,0.09f,0.98f );
	colors[ImGuiCol_ChildBg] = ImVec4( 0.09f,0.10f,0.13f,1.00f );
	colors[ImGuiCol_PopupBg] = ImVec4( 0.06f,0.07f,0.09f,0.98f );
	colors[ImGuiCol_Border] = ImVec4( 0.18f,0.20f,0.26f,1.00f );
	colors[ImGuiCol_FrameBg] = ImVec4( 0.12f,0.14f,0.19f,1.00f );
	colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.16f,0.19f,0.26f,1.00f );
	colors[ImGuiCol_FrameBgActive] = ImVec4( 0.22f,0.25f,0.35f,1.00f );
	colors[ImGuiCol_TitleBg] = ImVec4( 0.06f,0.07f,0.09f,1.00f );
	colors[ImGuiCol_TitleBgActive] = ImVec4( 0.09f,0.10f,0.13f,1.00f );
	colors[ImGuiCol_Button] = ImVec4( 0.00f,0.47f,0.85f,1.00f );
	colors[ImGuiCol_ButtonHovered] = ImVec4( 0.00f,0.58f,1.00f,1.00f );
	colors[ImGuiCol_ButtonActive] = ImVec4( 0.00f,0.38f,0.70f,1.00f );
	colors[ImGuiCol_Header] = ImVec4( 0.12f,0.14f,0.19f,1.00f );
	colors[ImGuiCol_HeaderHovered] = ImVec4( 0.00f,0.47f,0.85f,0.40f );
	colors[ImGuiCol_HeaderActive] = ImVec4( 0.00f,0.47f,0.85f,0.80f );
	colors[ImGuiCol_Tab] = ImVec4( 0.09f,0.10f,0.13f,1.00f );
	colors[ImGuiCol_TabHovered] = ImVec4( 0.00f,0.47f,0.85f,0.40f );
	colors[ImGuiCol_TabActive] = ImVec4( 0.00f,0.47f,0.85f,1.00f );
	colors[ImGuiCol_TabUnfocused] = ImVec4( 0.09f,0.10f,0.13f,1.00f );
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.12f,0.14f,0.19f,1.00f );

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\segoeui.ttf",20.0f );

	ImGui_ImplWin32_Init( window );
	ImGui_ImplDX11_Init( device,device_context );

	bool running = true;

	static ProcessInspector inspector;
	static char pidInput[32] = "";
	static char searchFilter[128] = "";

	while ( running ) {
		MSG msg;
		while ( PeekMessage( &msg,nullptr,0U,0U,PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if ( msg.message == WM_QUIT ) {
				running = false;
			}
		}

		if ( !running ) {
			break;
		}

		LONG_PTR exStyle = GetWindowLongPtr( window,GWL_EXSTYLE );
		if ( io.WantCaptureMouse ) {
			SetWindowLongPtr( window,GWL_EXSTYLE,exStyle & ~WS_EX_TRANSPARENT );
		}
		else {
			SetWindowLongPtr( window,GWL_EXSTYLE,exStyle | WS_EX_TRANSPARENT );
		}

		bool crosshair = false;
		bool show_menu = true;

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize( ImVec2( 1100,600 ) );

		ImGui::Begin( "Cromekks Inspector",&show_menu,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoTitleBar );

		if ( ImGui::IsWindowHovered() && ImGui::IsMouseDragging( ImGuiMouseButton_Left ) ) {
			ImVec2 delta = ImGui::GetIO().MouseDelta;
			RECT rect;
			GetWindowRect( window,&rect );
			MoveWindow( window,rect.left + delta.x,rect.top + delta.y,1920,1080,FALSE );
		}

		ImGui::SetCursorPos( ImVec2( 20.0f,15.0f ) );
		ImGui::TextColored( ImVec4( 0.00f,0.58f,1.00f,1.00f ),"CROMEKKS INSPECTOR" );

		ImGui::SetCursorPos( ImVec2( ImGui::GetWindowWidth() - 105.0f,12.0f ) );
		if ( ImGui::Button( "_",ImVec2( 40,28 ) ) ) {
			ShowWindow( window,SW_MINIMIZE );
		}
		ImGui::SameLine();
		ImGui::PushStyleColor( ImGuiCol_Button,ImVec4( 0.75f,0.15f,0.15f,1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered,ImVec4( 0.95f,0.25f,0.25f,1.0f ) );
		if ( ImGui::Button( "X",ImVec2( 40,28 ) ) ) {
			running = false;
		}
		ImGui::PopStyleColor( 2 );

		ImGui::SetCursorPos( ImVec2( 20.0f,60.0f ) );

		ImGui::Text( "Target Control:" );
		ImGui::SameLine();
		ImGui::SetNextItemWidth( 140.0f );
		ImGui::InputText( "PID",pidInput,IM_ARRAYSIZE( pidInput ),ImGuiInputTextFlags_CharsDecimal );

		ImGui::SameLine();
		if ( ImGui::Button( "Attach & Scan Suite" ) ) {
			if ( strlen( pidInput ) > 0 ) {
				targetPid = std::stoul( pidInput );
				targetHandle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,targetPid );

				if ( targetHandle && targetHandle != INVALID_HANDLE_VALUE ) {
					inspector.RefreshModules( targetPid );
					inspector.RefreshMemory( targetHandle );
					inspector.RefreshHandles( targetPid );
					CloseHandle( targetHandle );
				}
			}
		}

		ImGui::Separator();

		if ( ImGui::BeginTabBar( "InspectorTabBar" ) ) {

			if ( ImGui::BeginTabItem( "Modules & PE Headers" ) ) {
				if ( ImGui::BeginTable( "ModulesTable",4,ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,ImVec2( 0,420 ) ) ) {
					ImGui::TableSetupColumn( "Module Binary Name",ImGuiTableColumnFlags_WidthStretch );
					ImGui::TableSetupColumn( "Base Address",ImGuiTableColumnFlags_WidthFixed,180.0f );
					ImGui::TableSetupColumn( "Module Entry Point",ImGuiTableColumnFlags_WidthFixed,180.0f );
					ImGui::TableSetupColumn( "Size (Bytes)",ImGuiTableColumnFlags_WidthFixed,130.0f );
					ImGui::TableHeadersRow();

					for ( const auto& mod : inspector.modules ) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex( 0 ); ImGui::Text( "%ls",mod.name.c_str() );

						ImGui::TableSetColumnIndex( 1 );
						ImGui::Text( "0x%llX",mod.baseAddress );
						if ( ImGui::IsItemClicked() ) ImGui::SetClipboardText( std::to_string( mod.baseAddress ).c_str() );

						ImGui::TableSetColumnIndex( 2 );
						if ( mod.entryPoint != 0 ) ImGui::Text( "0x%llX",mod.entryPoint );
						else ImGui::TextDisabled( "N/A" );

						ImGui::TableSetColumnIndex( 3 ); ImGui::Text( "0x%X",mod.size );
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}

			if ( ImGui::BeginTabItem( "Memory Diagnostics Map" ) ) {
				ImGui::SetNextItemWidth( 250.0f );
				ImGui::InputText( "Filter Protection Flags",searchFilter,IM_ARRAYSIZE( searchFilter ) );
				ImGui::Spacing();

				if ( ImGui::BeginTable( "MemoryTable",5,ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,ImVec2( 0,360 ) ) ) {
					ImGui::TableSetupColumn( "Allocation Base",ImGuiTableColumnFlags_WidthFixed,160.0f );
					ImGui::TableSetupColumn( "Region Size",ImGuiTableColumnFlags_WidthFixed,140.0f );
					ImGui::TableSetupColumn( "State",ImGuiTableColumnFlags_WidthFixed,120.0f );
					ImGui::TableSetupColumn( "Type",ImGuiTableColumnFlags_WidthFixed,120.0f );
					ImGui::TableSetupColumn( "Protection",ImGuiTableColumnFlags_WidthStretch );
					ImGui::TableHeadersRow();

					for ( const auto& region : inspector.regions ) {
						if ( strlen( searchFilter ) > 0 && region.protectStr.find( searchFilter ) == std::string::npos )
							continue;

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex( 0 ); ImGui::Text( "0x%llX",region.baseAddress );
						if ( ImGui::IsItemClicked() ) ImGui::SetClipboardText( std::to_string( region.baseAddress ).c_str() );

						ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "0x%llX",region.size );
						ImGui::TableSetColumnIndex( 2 ); ImGui::Text( "%s",region.stateStr.c_str() );
						ImGui::TableSetColumnIndex( 3 ); ImGui::Text( "%s",region.typeStr.c_str() );

						ImGui::TableSetColumnIndex( 4 );
						if ( region.protectStr == "RWX" ) {
							ImGui::TextColored( ImVec4( 1.0f,0.3f,0.3f,1.0f ),"%s",region.protectStr.c_str() );
						}
						else if ( region.protectStr.find( "X" ) != std::string::npos ) {
							ImGui::TextColored( ImVec4( 0.3f,1.0f,0.3f,1.0f ),"%s",region.protectStr.c_str() );
						}
						else {
							ImGui::Text( "%s",region.protectStr.c_str() );
						}
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}

			if ( ImGui::BeginTabItem( "System Object Handles" ) ) {
				if ( ImGui::BeginTable( "HandlesTable",4,ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,ImVec2( 0,420 ) ) ) {
					ImGui::TableSetupColumn( "Handle ID",ImGuiTableColumnFlags_WidthFixed,100.0f );
					ImGui::TableSetupColumn( "Object Type",ImGuiTableColumnFlags_WidthFixed,140.0f );
					ImGui::TableSetupColumn( "Granted Access Mask",ImGuiTableColumnFlags_WidthFixed,180.0f );
					ImGui::TableSetupColumn( "Resource Path / Object Name",ImGuiTableColumnFlags_WidthStretch );
					ImGui::TableHeadersRow();

					for ( const auto& handle : inspector.handles ) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex( 0 ); ImGui::Text( "%d",handle.id );
						ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%s",handle.type.c_str() );
						ImGui::TableSetColumnIndex( 2 ); ImGui::Text( "0x%08X",handle.access );
						ImGui::TableSetColumnIndex( 3 ); ImGui::Text( "%s",handle.name.c_str() );
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		if ( crosshair == true )
		{
			ImGui::GetBackgroundDrawList()->AddCircleFilled( { 100,100 },10.f,ImColor( 1.f,1.f,1.f ) );
		}
		ImGui::End();

		ImGui::Render();

		constexpr float color[4]{ 0.f,0.f,0.f,0.f };
		device_context->OMSetRenderTargets( 1U,&render_target_view,nullptr );
		device_context->ClearRenderTargetView( render_target_view,color );

		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

		swap_chain->Present( 1U,0U );
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	if ( swap_chain ) {
		swap_chain->Release();
	}

	if ( device_context ) {
		device_context->Release();
	}

	if ( device ) {
		device->Release();
	}

	if ( render_target_view ) {
		render_target_view->Release();
	}

	DestroyWindow( window );
	UnregisterClassW( wc.lpszClassName,wc.hInstance );

	return 0;
}