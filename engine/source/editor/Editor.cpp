#include "editor/Editor.h"
#include "core/Scene.h"
#include "core/GameObject.h"
#include "core/Transform.h"
#include "core/SceneSerializer.h"
#include "graphics/Camera.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/SpriteBatch.h"
#include "physics/Rigidbody2D.h"
#include "physics/BoxCollider2D.h"
#include "physics/CircleCollider2D.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <cstring>

namespace eng
{
    Editor::Editor() = default;

    Editor::~Editor()
    {
        Shutdown();
    }

    bool Editor::Init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // 需要更新 ImGui 版本

        // 禁用 ini 文件，避免窗口位置缓存
        io.IniFilename = nullptr;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        std::cout << "ImGui 初始化完成" << std::endl;
        return true;
    }

    void Editor::Shutdown()
    {
        DestroyFramebuffer();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Editor::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Editor::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Editor::DrawMenuBar(Scene* scene)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::InputText("Path", m_SavePath, sizeof(m_SavePath));
                if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                {
                    SceneSerializer::Save(scene, m_SavePath);
                }
                if (ImGui::MenuItem("Load Scene", "Ctrl+L"))
                {
                    SceneSerializer::Load(scene, m_SavePath);
                    m_SelectedObject = nullptr;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                {
                    glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::MenuItem("Undo", "Ctrl+Z");
                ImGui::MenuItem("Redo", "Ctrl+Y");
                ImGui::Separator();
                ImGui::MenuItem("Preferences...");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("Scene Hierarchy", nullptr, true);
                ImGui::MenuItem("Inspector", nullptr, true);
                ImGui::MenuItem("Game View", nullptr, true);
                ImGui::EndMenu();
            }

            ImGui::Separator();
            if (ImGui::Button(m_IsRunning ? " Pause" : " Play"))
            {
                m_IsRunning = !m_IsRunning;
            }
            ImGui::SameLine();
            if (ImGui::Button(" Stop"))
            {
                m_IsRunning = false;
            }

            ImGui::EndMainMenuBar();
        }
    }

    void Editor::DrawSceneHierarchy(Scene* scene)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(280, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Scene Hierarchy");

        if (!scene)
        {
            ImGui::Text("No scene loaded");
            ImGui::End();
            return;
        }

        if (ImGui::Button("+ Create Empty"))
        {
            auto* go = scene->CreateGameObject("GameObject");
            m_SelectedObject = go;
        }
        ImGui::Separator();

        const auto& objects = scene->GetRootGameObjects();
        for (const auto& goPtr : objects)
        {
            GameObject* go = goPtr.get();
            if (!go) continue;

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                     | ImGuiTreeNodeFlags_SpanAvailWidth;

            if (go == m_SelectedObject)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool isOpen = ImGui::TreeNodeEx(go->GetName().c_str(), flags);

            if (ImGui::IsItemClicked())
            {
                m_SelectedObject = go;
            }

            if (isOpen)
            {
                Transform* transform = go->GetTransform();
                if (transform)
                {
                    const auto& children = transform->GetChildren();
                    for (auto* child : children)
                    {
                        if (!child || !child->GetGameObject()) continue;

                        ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_Leaf
                                                      | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                        if (child->GetGameObject() == m_SelectedObject)
                            childFlags |= ImGuiTreeNodeFlags_Selected;

                        ImGui::TreeNodeEx(child->GetGameObject()->GetName().c_str(), childFlags);
                        if (ImGui::IsItemClicked())
                        {
                            m_SelectedObject = child->GetGameObject();
                        }
                    }
                }
                ImGui::TreePop();
            }
        }

        if (m_SelectedObject && ImGui::Button("Delete Selected"))
        {
            scene->DestroyGameObject(m_SelectedObject);
            m_SelectedObject = nullptr;
        }

        ImGui::End();
    }

    void Editor::DrawInspector(GameObject* selected)
    {
        ImGui::SetNextWindowPos(ImVec2(280, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(320, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Inspector");

        if (!selected)
        {
            ImGui::Text("No object selected");
            ImGui::End();
            return;
        }

        char nameBuf[128];
        strncpy(nameBuf, selected->GetName().c_str(), sizeof(nameBuf) - 1);
        nameBuf[sizeof(nameBuf) - 1] = '\0';
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
        {
            selected->SetName(nameBuf);
        }

        ImGui::Separator();

        // Transform 组件
        ImGui::Text("Transform");
        Transform* transform = selected->GetTransform();
        if (transform)
        {
            glm::vec3 pos = transform->GetPosition();
            glm::vec3 euler = transform->GetEulerAngles();
            glm::vec3 scale = transform->GetScale();

            ImGui::DragFloat3("Position", &pos.x, 1.0f);
            ImGui::DragFloat3("Rotation", &euler.x, 1.0f);
            ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.01f, 100.0f);

            transform->SetPosition(pos);
            transform->SetEulerAngles(euler);
            transform->SetScale(scale);
        }

        ImGui::Separator();

        // 组件列表
        ImGui::Text("Components");
        auto components = selected->GetAllComponents();
        for (auto* comp : components)
        {
            if (!comp) continue;

            ImGui::BulletText("%s", comp->GetClassName());

            if (auto* sr = dynamic_cast<SpriteRenderer*>(comp))
            {
                ImGui::Indent();

                float color[4] = { sr->GetColor().x, sr->GetColor().y,
                                   sr->GetColor().z, sr->GetColor().w };
                if (ImGui::ColorEdit4("Color", color))
                {
                    sr->SetColor(color[0], color[1], color[2], color[3]);
                }

                glm::vec2 size = sr->GetSize();
                ImGui::DragFloat2("Size", &size.x, 1.0f, 1.0f, 10000.0f);
                sr->SetSize(size.x, size.y);

                int layer = sr->GetLayer();
                if (ImGui::DragInt("Layer", &layer))
                {
                    sr->SetLayer(layer);
                }

                int order = sr->GetOrderInLayer();
                if (ImGui::DragInt("Order In Layer", &order))
                {
                    sr->SetOrderInLayer(order);
                }

                ImGui::Unindent();
            }

            if (auto* cam = dynamic_cast<Camera*>(comp))
            {
                ImGui::Indent();

                int projType = static_cast<int>(cam->GetProjectionType());
                if (ImGui::Combo("Projection", &projType, "Orthographic\0Perspective\0"))
                {
                    if (projType == 0)
                        cam->SetOrthographic(cam->GetOrthoSize());
                    else
                        cam->SetPerspective(cam->GetFOV());
                }

                if (cam->GetProjectionType() == Camera::ProjectionType::Orthographic)
                {
                    float orthoSize = cam->GetOrthoSize();
                    if (ImGui::DragFloat("Ortho Size", &orthoSize, 1.0f, 1.0f, 10000.0f))
                    {
                        cam->SetOrthographic(orthoSize);
                    }
                }
                else
                {
                    float fov = cam->GetFOV();
                    if (ImGui::DragFloat("FOV", &fov, 1.0f, 1.0f, 179.0f))
                    {
                        cam->SetPerspective(fov);
                    }
                }

                ImGui::Unindent();
            }

            // Rigidbody2D 组件
            if (auto* rb = dynamic_cast<Rigidbody2D*>(comp))
            {
                ImGui::Indent();

                float mass = rb->GetMass();
                if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 1000.0f))
                {
                    rb->SetMass(mass);
                }

                bool kinematic = rb->IsKinematic();
                if (ImGui::Checkbox("Kinematic", &kinematic))
                {
                    rb->SetKinematic(kinematic);
                }

                glm::vec2 velocity = rb->GetVelocity();
                if (ImGui::DragFloat2("Velocity", &velocity.x, 1.0f))
                {
                    rb->SetVelocity(velocity);
                }

                float angularVel = rb->GetAngularVelocity();
                if (ImGui::DragFloat("Angular Velocity", &angularVel, 0.1f))
                {
                    rb->SetAngularVelocity(angularVel);
                }

                float linearDamping = rb->GetLinearDamping();
                if (ImGui::DragFloat("Linear Damping", &linearDamping, 0.01f, 0.0f, 10.0f))
                {
                    rb->SetLinearDamping(linearDamping);
                }

                float angularDamping = rb->GetAngularDamping();
                if (ImGui::DragFloat("Angular Damping", &angularDamping, 0.01f, 0.0f, 10.0f))
                {
                    rb->SetAngularDamping(angularDamping);
                }

                float gravityScale = rb->GetGravityScale();
                if (ImGui::DragFloat("Gravity Scale", &gravityScale, 0.1f, 0.0f, 10.0f))
                {
                    rb->SetGravityScale(gravityScale);
                }

                ImGui::Unindent();
            }

            // BoxCollider2D 组件
            if (auto* box = dynamic_cast<BoxCollider2D*>(comp))
            {
                ImGui::Indent();

                glm::vec2 size = box->GetSize();
                if (ImGui::DragFloat2("Size", &size.x, 1.0f, 0.1f, 1000.0f))
                {
                    box->SetSize(size);
                }

                glm::vec2 offset = box->GetOffset();
                if (ImGui::DragFloat2("Offset", &offset.x, 1.0f))
                {
                    box->SetOffset(offset);
                }

                bool isTrigger = box->IsTrigger();
                if (ImGui::Checkbox("Is Trigger", &isTrigger))
                {
                    box->SetTrigger(isTrigger);
                }

                ImGui::Unindent();
            }

            // CircleCollider2D 组件
            if (auto* circle = dynamic_cast<CircleCollider2D*>(comp))
            {
                ImGui::Indent();

                float radius = circle->GetRadius();
                if (ImGui::DragFloat("Radius", &radius, 1.0f, 0.1f, 1000.0f))
                {
                    circle->SetRadius(radius);
                }

                glm::vec2 offset = circle->GetOffset();
                if (ImGui::DragFloat2("Offset", &offset.x, 1.0f))
                {
                    circle->SetOffset(offset);
                }

                bool isTrigger = circle->IsTrigger();
                if (ImGui::Checkbox("Is Trigger", &isTrigger))
                {
                    circle->SetTrigger(isTrigger);
                }

                ImGui::Unindent();
            }
        }

        ImGui::End();
    }

    void Editor::DrawGameView(GLuint framebufferTexture, int width, int height)
    {
        ImGui::SetNextWindowPos(ImVec2(600, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Game View");

        if (framebufferTexture != 0)
        {
            ImVec2 availSize = ImGui::GetContentRegionAvail();
            float aspect = (float)width / (float)height;
            float viewW = availSize.x;
            float viewH = viewW / aspect;

            if (viewH > availSize.y)
            {
                viewH = availSize.y;
                viewW = viewH * aspect;
            }

            // 确保纹理绑定到 GL_TEXTURE0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebufferTexture);
            
            // 使用 ImGui::Image 显示纹理
            ImGui::Image((ImTextureID)(intptr_t)framebufferTexture,
                         ImVec2(viewW, viewH),
                         ImVec2(0, 1), ImVec2(1, 0));  // 翻转 Y 轴
        }
        else
        {
            ImGui::Text("No framebuffer texture");
        }

        ImGui::End();
    }

    void Editor::CreateFramebuffer(int width, int height)
    {
        DestroyFramebuffer();

        m_FramebufferWidth = width;
        m_FramebufferHeight = height;

        glGenFramebuffers(1, &m_Framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

        glGenTextures(1, &m_FramebufferTexture);
        glBindTexture(GL_TEXTURE_2D, m_FramebufferTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FramebufferTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Framebuffer incomplete" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Editor::DestroyFramebuffer()
    {
        if (m_FramebufferTexture != 0)
        {
            glDeleteTextures(1, &m_FramebufferTexture);
            m_FramebufferTexture = 0;
        }
        if (m_Framebuffer != 0)
        {
            glDeleteFramebuffers(1, &m_Framebuffer);
            m_Framebuffer = 0;
        }
    }
}