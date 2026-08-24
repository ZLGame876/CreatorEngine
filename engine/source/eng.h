#pragma once

#include "Application.h"
#include "CreatorEngine.h"
#include "input/InputManager.h"
#include "graphics/shaderprogram.h"

// 核心
#include "core/Object.h"
#include "core/Component.h"
#include "core/ProjectPaths.h"
#include "core/Transform.h"
#include "core/GameObject.h"
#include "core/Scene.h"
#include "core/Script.h"
#include "core/SceneSerializer.h"
#include "scripting/MonoRuntime.h"
#include "scripting/CSharpScript.h"

// 2D 渲染
#include "graphics/Texture.h"
#include "graphics/Framebuffer.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/SpriteBatch.h"
#include "graphics/Camera.h"
#include "physics/Rigidbody2D.h"
#include "physics/PhysicsMaterial.h"
#include "physics/Collider2D.h"
#include "physics/BoxCollider2D.h"
#include "physics/CircleCollider2D.h"
#include "physics/PhysicsWorld.h"
#include "gameplay/GameplayComponents.h"

// 编辑器
#include "editor/Editor.h"
