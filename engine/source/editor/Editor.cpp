#include "editor/Editor.h"

#include "core/Component.h"
#include "core/GameObject.h"
#include "core/ProjectPaths.h"
#include "core/Scene.h"
#include "core/SceneSerializer.h"
#include "core/Transform.h"
#include "graphics/Camera.h"
#include "graphics/SpriteRenderer.h"
#include "physics/BoxCollider2D.h"
#include "physics/CircleCollider2D.h"
#include "physics/Rigidbody2D.h"
#include "scripting/CSharpScript.h"
#include "scripting/MonoRuntime.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
    constexpr const char* GameObjectPayload = "CREATOR_GAME_OBJECT";

    void ShowTooltip(const char* text)
    {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        {
            ImGui::SetTooltip("%s", text);
        }
    }

    bool ModeButton(const char* label, bool selected, const ImVec2& size)
    {
        if (selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.43f, 0.72f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.50f, 0.82f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.37f, 0.64f, 1.0f));
        }
        const bool pressed = ImGui::Button(label, size);
        if (selected)
        {
            ImGui::PopStyleColor(3);
        }
        return pressed;
    }

    void DrawDirectory(const std::filesystem::path& path, int depth)
    {
        if (depth > 6)
        {
            return;
        }

        std::error_code error;
        std::vector<std::filesystem::directory_entry> entries;
        for (std::filesystem::directory_iterator it(path, error), end; !error && it != end;
             it.increment(error))
        {
            entries.push_back(*it);
        }
        std::sort(entries.begin(), entries.end(),
            [](const auto& left, const auto& right) {
                if (left.is_directory() != right.is_directory())
                {
                    return left.is_directory();
                }
                return left.path().filename().string() < right.path().filename().string();
            });

        for (const auto& entry : entries)
        {
            const std::string name = entry.path().filename().string();
            if (entry.is_directory())
            {
                const bool open = ImGui::TreeNodeEx(entry.path().string().c_str(),
                    ImGuiTreeNodeFlags_SpanAvailWidth, "%s", name.c_str());
                if (open)
                {
                    DrawDirectory(entry.path(), depth + 1);
                    ImGui::TreePop();
                }
            }
            else
            {
                ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
            }
        }
    }
}

namespace eng
{
    Editor::Editor() = default;

    Editor::~Editor()
    {
        Shutdown();
    }

    bool Editor::Init(GLFWwindow* window)
    {
        if (m_IsInitialized)
        {
            return true;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        m_IsShutdown = false;

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;

        ApplyStyle();

        if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
        {
            Shutdown();
            return false;
        }
        if (!ImGui_ImplOpenGL3_Init("#version 330"))
        {
            Shutdown();
            return false;
        }

        m_IsInitialized = true;
        AddConsoleMessage("CreatorEngine editor initialized");
        std::cout << "ImGui editor initialized" << std::endl;
        return true;
    }

    void Editor::Shutdown()
    {
        if (m_IsShutdown)
        {
            return;
        }

        if (m_IsInitialized)
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
        }
        if (ImGui::GetCurrentContext())
        {
            ImGui::DestroyContext();
        }

        m_IsInitialized = false;
        m_IsShutdown = true;
    }

    void Editor::ApplyStyle()
    {
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.ChildRounding = 3.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 3.0f;
        style.TabRounding = 3.0f;
        style.ScrollbarRounding = 3.0f;
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.WindowPadding = ImVec2(7.0f, 7.0f);
        style.FramePadding = ImVec2(7.0f, 4.0f);
        style.ItemSpacing = ImVec2(6.0f, 5.0f);

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.105f, 0.110f, 0.120f, 1.0f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.125f, 0.130f, 0.140f, 1.0f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.145f, 0.150f, 0.160f, 1.0f);
        colors[ImGuiCol_Border] = ImVec4(0.235f, 0.245f, 0.260f, 1.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.180f, 0.188f, 0.200f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.235f, 0.255f, 0.275f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.120f, 0.410f, 0.690f, 1.0f);
        colors[ImGuiCol_Button] = ImVec4(0.205f, 0.215f, 0.230f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.285f, 0.300f, 0.320f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.120f, 0.410f, 0.690f, 1.0f);
        colors[ImGuiCol_Header] = ImVec4(0.185f, 0.200f, 0.215f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.250f, 0.275f, 0.300f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.120f, 0.410f, 0.690f, 1.0f);
        colors[ImGuiCol_Tab] = ImVec4(0.145f, 0.150f, 0.160f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.230f, 0.260f, 0.290f, 1.0f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.205f, 0.220f, 0.240f, 1.0f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.250f, 0.650f, 0.950f, 1.0f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.900f, 0.565f, 0.180f, 1.0f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.900f, 0.565f, 0.180f, 0.30f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.900f, 0.565f, 0.180f, 0.70f);
    }

    void Editor::BeginFrame()
    {
        if (!m_IsInitialized) return;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Editor::EndFrame()
    {
        if (!m_IsInitialized) return;
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Editor::DrawLayout(Scene* scene,
                            GLuint sceneTexture, int sceneTextureWidth, int sceneTextureHeight,
                            GLuint gameTexture, int gameTextureWidth, int gameTextureHeight)
    {
        if (!m_IsInitialized)
        {
            return;
        }

        if (m_SelectedObject && (!scene || !scene->Contains(m_SelectedObject)))
        {
            m_SelectedObject = nullptr;
        }

        DrawMenuBar(scene);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        const ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoDecoration |
                                           ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGui::Begin("CreatorEngine##Workspace", nullptr, rootFlags);
        DrawToolbar();
        ImGui::Separator();

        ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable |
                                     ImGuiTableFlags_BordersInnerV |
                                     ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("##EditorColumns", 3, tableFlags, ImGui::GetContentRegionAvail()))
        {
            ImGui::TableSetupColumn("Hierarchy", ImGuiTableColumnFlags_WidthFixed,
                                    m_ShowHierarchy ? 245.0f : 1.0f);
            ImGui::TableSetupColumn("Workspace", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthFixed,
                                    m_ShowInspector ? 310.0f : 1.0f);

            ImGui::TableNextColumn();
            if (m_ShowHierarchy)
            {
                ImGui::BeginChild("##HierarchyPanel", ImVec2(0, 0), false);
                DrawSceneHierarchy(scene);
                ImGui::EndChild();
            }

            ImGui::TableNextColumn();
            ImGui::BeginChild("##CenterPanel", ImVec2(0, 0), false);
            const float bottomHeight = (m_ShowProject || m_ShowConsole) ? 190.0f : 0.0f;
            const float viewportHeight = std::max(140.0f,
                ImGui::GetContentRegionAvail().y - bottomHeight - (bottomHeight > 0.0f ? 5.0f : 0.0f));
            ImGui::BeginChild("##ViewportPanel", ImVec2(0, viewportHeight), true,
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            DrawViewportTabs(sceneTexture, sceneTextureWidth, sceneTextureHeight,
                             gameTexture, gameTextureWidth, gameTextureHeight);
            ImGui::EndChild();
            if (bottomHeight > 0.0f)
            {
                ImGui::BeginChild("##BottomPanel", ImVec2(0, 0), true);
                DrawBottomPanel();
                ImGui::EndChild();
            }
            ImGui::EndChild();

            ImGui::TableNextColumn();
            if (m_ShowInspector)
            {
                ImGui::BeginChild("##InspectorPanel", ImVec2(0, 0), false);
                DrawInspector(m_SelectedObject);
                ImGui::EndChild();
            }

            ImGui::EndTable();
        }

        ImGui::End();
        ProcessDeferredActions(scene);
    }

    void Editor::DrawMenuBar(Scene* scene)
    {
        if (!ImGui::BeginMainMenuBar())
        {
            return;
        }

        ImGui::TextUnformatted("CreatorEngine");
        ImGui::Separator();

        if (ImGui::BeginMenu("File"))
        {
            ImGui::SetNextItemWidth(220.0f);
            ImGui::InputText("Scene", m_SavePath, sizeof(m_SavePath));
            if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, scene != nullptr))
            {
                const bool saved = SceneSerializer::Save(scene, m_SavePath);
                AddConsoleMessage(saved ? "Saved scene: " + std::string(m_SavePath)
                                        : "Failed to save scene: " + std::string(m_SavePath));
            }
            if (ImGui::MenuItem("Load Scene", "Ctrl+L", false, scene != nullptr))
            {
                const bool loaded = SceneSerializer::Load(scene, m_SavePath);
                m_SelectedObject = nullptr;
                AddConsoleMessage(loaded ? "Loaded scene: " + std::string(m_SavePath)
                                         : "Failed to load scene: " + std::string(m_SavePath));
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("GameObject"))
        {
            if (ImGui::MenuItem("Create Empty", nullptr, false, scene != nullptr))
            {
                m_SelectedObject = scene->CreateGameObject("GameObject");
            }
            if (ImGui::MenuItem("Create Child", nullptr, false,
                                scene != nullptr && m_SelectedObject != nullptr))
            {
                GameObject* child = scene->CreateGameObject("GameObject");
                child->SetParent(m_SelectedObject, false);
                m_SelectedObject = child;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del", false, m_SelectedObject != nullptr))
            {
                m_PendingDelete = m_SelectedObject;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &m_ShowInspector);
            ImGui::MenuItem("Project", nullptr, &m_ShowProject);
            ImGui::MenuItem("Console", nullptr, &m_ShowConsole);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    void Editor::DrawToolbar()
    {
        const float buttonWidth = 34.0f;
        const float groupWidth = buttonWidth * 3.0f + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(),
            (ImGui::GetWindowWidth() - groupWidth) * 0.5f));

        const bool playing = m_PlayState == PlayState::Playing;
        if (ModeButton(">", playing, ImVec2(buttonWidth, 25.0f)))
        {
            m_PlayState = PlayState::Playing;
            AddConsoleMessage("Play mode started");
        }
        ShowTooltip("Play");

        ImGui::SameLine();
        const bool paused = m_PlayState == PlayState::Paused;
        if (ModeButton("||", paused, ImVec2(buttonWidth, 25.0f)) &&
            m_PlayState != PlayState::Editing)
        {
            m_PlayState = paused ? PlayState::Playing : PlayState::Paused;
        }
        ShowTooltip("Pause");

        ImGui::SameLine();
        ImGui::BeginDisabled(m_PlayState == PlayState::Editing);
        if (ImGui::Button("[]", ImVec2(buttonWidth, 25.0f)))
        {
            m_PlayState = PlayState::Editing;
            AddConsoleMessage("Play mode stopped");
        }
        ImGui::EndDisabled();
        ShowTooltip("Stop");
    }

    void Editor::DrawSceneHierarchy(Scene* scene)
    {
        ImGui::TextUnformatted("Hierarchy");
        ImGui::SameLine();
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), ImGui::GetWindowWidth() - 30.0f));
        if (ImGui::Button("+", ImVec2(22.0f, 22.0f)) && scene)
        {
            m_SelectedObject = scene->CreateGameObject("GameObject");
        }
        ShowTooltip("Create GameObject");
        ImGui::Separator();

        if (!scene)
        {
            ImGui::TextDisabled("No scene");
            return;
        }

        ImGui::PushID(scene);
        const ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen |
                                              ImGuiTreeNodeFlags_SpanAvailWidth |
                                              ImGuiTreeNodeFlags_OpenOnArrow;
        const bool sceneOpen = ImGui::TreeNodeEx("##SceneRoot", sceneFlags, "%s", scene->GetName().c_str());
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GameObjectPayload))
            {
                m_PendingReparentObject = *static_cast<GameObject* const*>(payload->Data);
                m_PendingParent = nullptr;
            }
            ImGui::EndDragDropTarget();
        }
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create Empty"))
            {
                m_SelectedObject = scene->CreateGameObject("GameObject");
            }
            ImGui::EndPopup();
        }

        if (sceneOpen)
        {
            for (GameObject* object : scene->GetRootGameObjects())
            {
                DrawGameObjectNode(object, scene);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
            !ImGui::IsAnyItemHovered())
        {
            m_SelectedObject = nullptr;
        }
    }

    void Editor::DrawGameObjectNode(GameObject* object, Scene* scene)
    {
        if (!object)
        {
            return;
        }

        ImGui::PushID(object);
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;
        if (object->GetChildren().empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        if (object == m_SelectedObject)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        const bool open = ImGui::TreeNodeEx("##Object", flags, "%s", object->GetName().c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectedObject = object;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            GameObject* payloadObject = object;
            ImGui::SetDragDropPayload(GameObjectPayload, &payloadObject, sizeof(payloadObject));
            ImGui::TextUnformatted(object->GetName().c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GameObjectPayload))
            {
                m_PendingReparentObject = *static_cast<GameObject* const*>(payload->Data);
                m_PendingParent = object;
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create Child"))
            {
                GameObject* child = scene->CreateGameObject("GameObject");
                child->SetParent(object, false);
                m_SelectedObject = child;
            }
            if (ImGui::MenuItem("Delete"))
            {
                m_PendingDelete = object;
            }
            ImGui::EndPopup();
        }

        if (open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
        {
            const std::vector<Transform*> children = object->GetChildren();
            for (Transform* child : children)
            {
                if (child)
                {
                    DrawGameObjectNode(child->GetGameObject(), scene);
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void Editor::DrawInspector(GameObject* selected)
    {
        ImGui::TextUnformatted("Inspector");
        ImGui::Separator();

        if (!selected)
        {
            ImGui::TextDisabled("No selection");
            return;
        }

        bool active = selected->IsActiveSelf();
        if (ImGui::Checkbox("##Active", &active))
        {
            selected->SetActive(active);
        }
        ShowTooltip("Active");
        ImGui::SameLine();

        char nameBuffer[128];
        std::strncpy(nameBuffer, selected->GetName().c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer)))
        {
            selected->SetName(nameBuffer);
        }
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            Transform* transform = selected->GetTransform();
            glm::vec3 position = transform->GetPosition();
            glm::vec3 rotation = transform->GetEulerAngles();
            glm::vec3 scale = transform->GetScale();
            if (ImGui::DragFloat3("Position", &position.x, 0.1f)) transform->SetPosition(position);
            if (ImGui::DragFloat3("Rotation", &rotation.x, 0.25f)) transform->SetEulerAngles(rotation);
            if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.001f, 1000.0f)) transform->SetScale(scale);
        }

        for (Component* component : selected->GetAllComponents())
        {
            if (!component || component == selected->GetTransform())
            {
                continue;
            }
            DrawComponentInspector(selected, component);
        }

        ImGui::Spacing();
        const float width = std::min(220.0f, ImGui::GetContentRegionAvail().x);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
            std::max(0.0f, (ImGui::GetContentRegionAvail().x - width) * 0.5f));
        if (ImGui::Button("Add Component", ImVec2(width, 0.0f)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }
        DrawAddComponentMenu(selected);
    }

    void Editor::DrawComponentInspector(GameObject* selected, Component* component)
    {
        ImGui::PushID(component);
        const bool open = ImGui::CollapsingHeader(component->GetClassName(),
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
        ImGui::SameLine(ImGui::GetWindowWidth() - 31.0f);
        if (ImGui::SmallButton("x"))
        {
            m_PendingRemoveComponent = component;
        }
        ShowTooltip("Remove component");

        if (open)
        {
            if (auto* renderer = dynamic_cast<SpriteRenderer*>(component))
            {
                glm::vec4 color = renderer->GetColor();
                glm::vec2 size = renderer->GetSize();
                int layer = renderer->GetLayer();
                int order = renderer->GetOrderInLayer();
                if (ImGui::ColorEdit4("Color", &color.x)) renderer->SetColor(color);
                if (ImGui::DragFloat2("Size", &size.x, 0.1f, 0.001f, 100000.0f)) renderer->SetSize(size);
                if (ImGui::DragInt("Layer", &layer)) renderer->SetLayer(layer);
                if (ImGui::DragInt("Order", &order)) renderer->SetOrderInLayer(order);
            }
            else if (auto* camera = dynamic_cast<Camera*>(component))
            {
                int projection = static_cast<int>(camera->GetProjectionType());
                if (ImGui::Combo("Projection", &projection, "Orthographic\0Perspective\0"))
                {
                    if (projection == 0)
                        camera->SetOrthographic(camera->GetOrthoSize(), camera->GetNearPlane(), camera->GetFarPlane());
                    else
                        camera->SetPerspective(camera->GetFOV(), std::max(0.001f, camera->GetNearPlane()), camera->GetFarPlane());
                }
                if (projection == 0)
                {
                    float size = camera->GetOrthoSize();
                    if (ImGui::DragFloat("Size", &size, 0.1f, 0.001f, 100000.0f))
                        camera->SetOrthographic(size, camera->GetNearPlane(), camera->GetFarPlane());
                }
                else
                {
                    float fov = camera->GetFOV();
                    if (ImGui::DragFloat("Field of View", &fov, 0.25f, 1.0f, 179.0f))
                        camera->SetPerspective(fov, camera->GetNearPlane(), camera->GetFarPlane());
                }
            }
            else if (auto* script = dynamic_cast<CSharpScript*>(component))
            {
                char assembly[256];
                char nameSpace[128];
                char className[128];
                std::strncpy(assembly, script->GetAssemblyPath().c_str(), sizeof(assembly) - 1);
                std::strncpy(nameSpace, script->GetNamespaceName().c_str(), sizeof(nameSpace) - 1);
                std::strncpy(className, script->GetManagedClassName().c_str(), sizeof(className) - 1);
                assembly[sizeof(assembly) - 1] = '\0';
                nameSpace[sizeof(nameSpace) - 1] = '\0';
                className[sizeof(className) - 1] = '\0';
                if (ImGui::InputText("Assembly", assembly, sizeof(assembly))) script->SetAssemblyPath(assembly);
                if (ImGui::InputText("Namespace", nameSpace, sizeof(nameSpace))) script->SetNamespaceName(nameSpace);
                if (ImGui::InputText("Class", className, sizeof(className))) script->SetClassName(className);
                if (ImGui::Button("Reload")) script->Reload();
                ImGui::SameLine();
                if (script->IsLoaded())
                {
                    ImGui::TextColored(ImVec4(0.30f, 0.82f, 0.48f, 1.0f), "Loaded");
                }
                else if (!script->GetLoadError().empty())
                {
                    ImGui::TextColored(ImVec4(0.95f, 0.38f, 0.32f, 1.0f), "Not loaded");
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", script->GetLoadError().c_str());
                    }
                }
                else
                {
                    ImGui::TextDisabled(MonoRuntime::GetInstance().IsAvailable() ? "Ready" : "Mono disabled");
                }
            }
            else if (auto* body = dynamic_cast<Rigidbody2D*>(component))
            {
                float mass = body->GetMass();
                bool kinematic = body->IsKinematic();
                float gravity = body->GetGravityScale();
                float linearDamping = body->GetLinearDamping();
                float angularDamping = body->GetAngularDamping();
                if (ImGui::DragFloat("Mass", &mass, 0.05f, 0.001f, 100000.0f)) body->SetMass(mass);
                if (ImGui::Checkbox("Kinematic", &kinematic)) body->SetKinematic(kinematic);
                if (ImGui::DragFloat("Gravity Scale", &gravity, 0.05f, -100.0f, 100.0f)) body->SetGravityScale(gravity);
                if (ImGui::DragFloat("Linear Damping", &linearDamping, 0.01f, 0.0f, 100.0f)) body->SetLinearDamping(linearDamping);
                if (ImGui::DragFloat("Angular Damping", &angularDamping, 0.01f, 0.0f, 100.0f)) body->SetAngularDamping(angularDamping);
            }
            else if (auto* box = dynamic_cast<BoxCollider2D*>(component))
            {
                glm::vec2 size = box->GetSize();
                glm::vec2 offset = box->GetOffset();
                bool trigger = box->IsTrigger();
                if (ImGui::DragFloat2("Size", &size.x, 0.05f, 0.001f, 100000.0f)) box->SetSize(size);
                if (ImGui::DragFloat2("Offset", &offset.x, 0.05f)) box->SetOffset(offset);
                if (ImGui::Checkbox("Is Trigger", &trigger)) box->SetTrigger(trigger);
            }
            else if (auto* circle = dynamic_cast<CircleCollider2D*>(component))
            {
                float radius = circle->GetRadius();
                glm::vec2 offset = circle->GetOffset();
                bool trigger = circle->IsTrigger();
                if (ImGui::DragFloat("Radius", &radius, 0.05f, 0.001f, 100000.0f)) circle->SetRadius(radius);
                if (ImGui::DragFloat2("Offset", &offset.x, 0.05f)) circle->SetOffset(offset);
                if (ImGui::Checkbox("Is Trigger", &trigger)) circle->SetTrigger(trigger);
            }
            else
            {
                ImGui::TextDisabled("No editable properties");
            }
        }
        ImGui::PopID();
        (void)selected;
    }

    void Editor::DrawAddComponentMenu(GameObject* selected)
    {
        if (!ImGui::BeginPopup("AddComponentPopup"))
        {
            return;
        }

        if (ImGui::MenuItem("Sprite Renderer")) selected->AddComponent<SpriteRenderer>();
        if (ImGui::MenuItem("Camera", nullptr, false, selected->GetComponent<Camera>() == nullptr))
            selected->AddComponent<Camera>();
        if (ImGui::MenuItem("C# Script")) selected->AddComponent<CSharpScript>();
        ImGui::Separator();
        if (ImGui::MenuItem("Rigidbody 2D", nullptr, false, selected->GetComponent<Rigidbody2D>() == nullptr))
            selected->AddComponent<Rigidbody2D>();
        if (ImGui::MenuItem("Box Collider 2D")) selected->AddComponent<BoxCollider2D>();
        if (ImGui::MenuItem("Circle Collider 2D")) selected->AddComponent<CircleCollider2D>();

        ImGui::EndPopup();
    }

    void Editor::DrawViewportTabs(GLuint sceneTexture, int sceneTextureWidth, int sceneTextureHeight,
                                  GLuint gameTexture, int gameTextureWidth, int gameTextureHeight)
    {
        if (!ImGui::BeginTabBar("##ViewportTabs", ImGuiTabBarFlags_None))
        {
            return;
        }
        if (ImGui::BeginTabItem("Scene"))
        {
            DrawSceneView(sceneTexture, sceneTextureWidth, sceneTextureHeight);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Game"))
        {
            DrawGameView(gameTexture, gameTextureWidth, gameTextureHeight);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    void Editor::DrawSceneView(GLuint texture, int width, int height)
    {
        const bool is2D = m_SceneViewMode == SceneViewMode::Mode2D;
        if (ModeButton("2D", is2D, ImVec2(42.0f, 23.0f)))
        {
            m_SceneViewMode = SceneViewMode::Mode2D;
        }
        ImGui::SameLine(0.0f, 0.0f);
        if (ModeButton("3D", !is2D, ImVec2(42.0f, 23.0f)))
        {
            m_SceneViewMode = SceneViewMode::Mode3D;
        }
        ImGui::SameLine();
        if (ImGui::Button("F", ImVec2(28.0f, 23.0f)))
        {
            FocusSelection();
        }
        ShowTooltip("Frame selected");

        ImVec2 size = ImGui::GetContentRegionAvail();
        size.x = std::max(size.x, 1.0f);
        size.y = std::max(size.y, 1.0f);
        m_SceneViewportSize = glm::ivec2(static_cast<int>(size.x), static_cast<int>(size.y));

        if (texture != 0 && width > 0 && height > 0)
        {
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture)), size,
                         ImVec2(0, 1), ImVec2(1, 0));
            HandleSceneViewInput();
        }
    }

    void Editor::DrawGameView(GLuint texture, int width, int height)
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.x = std::max(size.x, 1.0f);
        size.y = std::max(size.y, 1.0f);
        m_GameViewportSize = glm::ivec2(static_cast<int>(size.x), static_cast<int>(size.y));
        if (texture != 0 && width > 0 && height > 0)
        {
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture)), size,
                         ImVec2(0, 1), ImVec2(1, 0));
        }
    }

    void Editor::DrawBottomPanel()
    {
        if (!ImGui::BeginTabBar("##BottomTabs"))
        {
            return;
        }
        if (m_ShowProject && ImGui::BeginTabItem("Project"))
        {
            DrawProjectPanel();
            ImGui::EndTabItem();
        }
        if (m_ShowConsole && ImGui::BeginTabItem("Console"))
        {
            DrawConsolePanel();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    void Editor::DrawProjectPanel()
    {
        const std::filesystem::path assets = ProjectPaths::ResolveResource("Assets");

        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
        if (ImGui::TreeNodeEx("##Assets", flags, "Assets"))
        {
            if (std::filesystem::exists(assets))
            {
                DrawDirectory(assets, 0);
            }
            ImGui::TreePop();
        }
    }

    void Editor::DrawConsolePanel()
    {
        if (ImGui::SmallButton("Clear"))
        {
            m_ConsoleMessages.clear();
        }
        ImGui::Separator();
        for (const std::string& message : m_ConsoleMessages)
        {
            const bool failed = message.find("Failed") != std::string::npos;
            if (failed) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.38f, 0.32f, 1.0f));
            ImGui::TextUnformatted(message.c_str());
            if (failed) ImGui::PopStyleColor();
        }
    }

    void Editor::HandleSceneViewInput()
    {
        if (!ImGui::IsItemHovered())
        {
            return;
        }

        ImGuiIO& io = ImGui::GetIO();
        if (io.MouseWheel != 0.0f)
        {
            const float zoom = std::pow(0.85f, io.MouseWheel);
            if (m_SceneViewMode == SceneViewMode::Mode2D)
                m_Scene2DOrthoSize = std::clamp(m_Scene2DOrthoSize * zoom, 0.01f, 100000.0f);
            else
                m_Scene3DDistance = std::clamp(m_Scene3DDistance * zoom, 0.1f, 100000.0f);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            const ImVec2 delta = io.MouseDelta;
            if (m_SceneViewMode == SceneViewMode::Mode2D)
            {
                const float unitsPerPixel = (m_Scene2DOrthoSize * 2.0f) /
                    static_cast<float>(std::max(1, m_SceneViewportSize.y));
                m_Scene2DCenter.x -= delta.x * unitsPerPixel;
                m_Scene2DCenter.y += delta.y * unitsPerPixel;
            }
            else
            {
                const float unitsPerPixel = m_Scene3DDistance /
                    static_cast<float>(std::max(1, m_SceneViewportSize.y));
                m_Scene3DPivot -= GetSceneCameraRight() * (delta.x * unitsPerPixel);
                m_Scene3DPivot += GetSceneCameraUp() * (delta.y * unitsPerPixel);
            }
        }

        if (m_SceneViewMode == SceneViewMode::Mode3D &&
            ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            m_Scene3DYaw += io.MouseDelta.x * 0.25f;
            m_Scene3DPitch = std::clamp(m_Scene3DPitch - io.MouseDelta.y * 0.25f,
                                        -89.0f, 89.0f);
        }
    }

    void Editor::FocusSelection()
    {
        if (!m_SelectedObject || !m_SelectedObject->GetTransform())
        {
            return;
        }
        const glm::vec3 position = m_SelectedObject->GetTransform()->GetWorldPosition();
        if (m_SceneViewMode == SceneViewMode::Mode2D)
        {
            m_Scene2DCenter = glm::vec2(position);
        }
        else
        {
            m_Scene3DPivot = position;
        }
    }

    void Editor::ProcessDeferredActions(Scene* scene)
    {
        if (!scene)
        {
            m_PendingDelete = nullptr;
            m_PendingReparentObject = nullptr;
            m_PendingParent = nullptr;
            m_PendingRemoveComponent = nullptr;
            return;
        }

        if (m_PendingReparentObject && scene->Contains(m_PendingReparentObject) &&
            (!m_PendingParent || scene->Contains(m_PendingParent)))
        {
            if (!m_PendingReparentObject->SetParent(m_PendingParent, true))
            {
                AddConsoleMessage("Rejected cyclic hierarchy change");
            }
        }
        m_PendingReparentObject = nullptr;
        m_PendingParent = nullptr;

        if (m_PendingRemoveComponent && m_SelectedObject && scene->Contains(m_SelectedObject))
        {
            m_SelectedObject->RemoveComponent(m_PendingRemoveComponent);
        }
        m_PendingRemoveComponent = nullptr;

        if (m_PendingDelete && scene->Contains(m_PendingDelete))
        {
            if (m_SelectedObject == m_PendingDelete)
            {
                m_SelectedObject = nullptr;
            }
            scene->DestroyGameObject(m_PendingDelete);
        }
        m_PendingDelete = nullptr;
    }

    glm::vec3 Editor::GetSceneCameraPosition() const
    {
        if (m_SceneViewMode == SceneViewMode::Mode2D)
        {
            return glm::vec3(m_Scene2DCenter, 10.0f);
        }

        const float yaw = glm::radians(m_Scene3DYaw);
        const float pitch = glm::radians(m_Scene3DPitch);
        const glm::vec3 offset(
            std::cos(pitch) * std::cos(yaw),
            std::sin(pitch),
            std::cos(pitch) * std::sin(yaw));
        return m_Scene3DPivot + offset * m_Scene3DDistance;
    }

    glm::vec3 Editor::GetSceneCameraForward() const
    {
        if (m_SceneViewMode == SceneViewMode::Mode2D)
        {
            return glm::vec3(0.0f, 0.0f, -1.0f);
        }
        return glm::normalize(m_Scene3DPivot - GetSceneCameraPosition());
    }

    glm::vec3 Editor::GetSceneCameraRight() const
    {
        return glm::normalize(glm::cross(GetSceneCameraForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 Editor::GetSceneCameraUp() const
    {
        return glm::normalize(glm::cross(GetSceneCameraRight(), GetSceneCameraForward()));
    }

    glm::mat4 Editor::GetSceneViewMatrix() const
    {
        const glm::vec3 position = GetSceneCameraPosition();
        return glm::lookAt(position, position + GetSceneCameraForward(), GetSceneCameraUp());
    }

    glm::mat4 Editor::GetSceneProjectionMatrix(float aspectRatio) const
    {
        aspectRatio = std::max(aspectRatio, 0.001f);
        if (m_SceneViewMode == SceneViewMode::Mode2D)
        {
            const float halfHeight = m_Scene2DOrthoSize;
            const float halfWidth = halfHeight * aspectRatio;
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -10000.0f, 10000.0f);
        }
        return glm::perspective(glm::radians(m_SceneFov), aspectRatio, 0.01f, 100000.0f);
    }

    glm::mat4 Editor::GetSceneViewProjectionMatrix(float aspectRatio) const
    {
        return GetSceneProjectionMatrix(aspectRatio) * GetSceneViewMatrix();
    }

    void Editor::SetSceneView2DFrame(const glm::vec2& center, float orthoSize)
    {
        m_Scene2DCenter = center;
        m_Scene2DOrthoSize = std::max(0.01f, orthoSize);
    }

    void Editor::AddConsoleMessage(const std::string& message)
    {
        m_ConsoleMessages.push_back(message);
        if (m_ConsoleMessages.size() > 500)
        {
            m_ConsoleMessages.erase(m_ConsoleMessages.begin(), m_ConsoleMessages.begin() + 100);
        }
    }
}
