# CreatorEngine playable vertical slice

CreatorEngine now contains a small but complete 2D platform-game slice in `source/Game.cpp`. It is intentionally built from engine primitives so the sample exercises the same APIs a game project would use.

## Runtime composition

```text
Scene
|- Player
|  |- SpriteRenderer
|  |- Rigidbody2D
|  |- BoxCollider2D
|  |- CharacterController2D
|  `- HealthComponent
|- Ground and five static platforms
|- Moving Platform (kinematic Rigidbody2D + Patrol2D)
|- Patrol Hazard (trigger BoxCollider2D + Hazard2D + Patrol2D)
|- Goal (trigger BoxCollider2D + Goal2D)
|- Death Plane (trigger BoxCollider2D + Hazard2D)
`- Main Camera (Camera + CameraFollow2D)
```

## Controls

Press Play in the editor toolbar. `A`/`D` or the arrow keys move, `Space` jumps, and `R` respawns the player. Touching a hazard resets the player; reaching the gold goal pauses gameplay and displays a win message in the Game view.

## Engine capabilities demonstrated

- Component composition and lifecycle (`Start`/`Update`) in a real scene.
- AABB collision resolution, gravity, friction, kinematic bodies, and trigger callbacks.
- Input polling through `CreatorEngine::GetInstance().GetInputManager()`.
- Camera following with world bounds and independent Scene/Game views.
- Reusable gameplay components that can also be added from the Inspector.

This slice is intentionally scoped. CreatorEngine still needs 3D meshes and materials, prefab/undo systems, production asset importing and packaging, Mono domain hot reload, and a visual animation editor before it should be considered a full production engine.
