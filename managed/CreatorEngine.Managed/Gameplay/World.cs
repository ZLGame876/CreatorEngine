using System;
using System.Collections.Generic;

namespace CreatorEngine
{
    public sealed class World : CreatorObject
    {
        private readonly List<Actor> _actors = new List<Actor>();
        private bool _hasBegunPlay;

        public World(string? name = null) : base(name ?? "World") { }

        public IReadOnlyList<Actor> Actors => _actors;
        public GameMode? GameMode { get; internal set; }
        public PlayerController? PrimaryPlayerController { get; internal set; }
        public Pawn? DefaultPawn { get; internal set; }
        public bool HasBegunPlay => _hasBegunPlay;

        public T SpawnActor<T>(string? name = null) where T : Actor, new()
        {
            return (T)SpawnActor(typeof(T), name);
        }

        public Actor SpawnActor(Type actorType, string? name = null)
        {
            ThrowIfDestroyed();
            if (actorType == null || !typeof(Actor).IsAssignableFrom(actorType) || actorType.IsAbstract)
            {
                throw new ArgumentException("The requested type must be a concrete Actor.", nameof(actorType));
            }

            var actor = (Actor?)Activator.CreateInstance(actorType);
            if (actor == null)
            {
                throw new InvalidOperationException($"Could not construct {actorType.FullName}.");
            }

            if (!string.IsNullOrWhiteSpace(name))
            {
                actor.Name = name!;
            }
            actor.World = this;
            _actors.Add(actor);
            if (_hasBegunPlay)
            {
                actor.BeginPlayInternal();
            }
            return actor;
        }

        public void BeginPlay()
        {
            if (_hasBegunPlay)
            {
                return;
            }
            _hasBegunPlay = true;
            foreach (Actor actor in new List<Actor>(_actors))
            {
                actor.BeginPlayInternal();
            }
        }

        public void Tick(float deltaTime)
        {
            if (!_hasBegunPlay)
            {
                return;
            }

            foreach (Actor actor in new List<Actor>(_actors))
            {
                if (!actor.IsDestroyed && actor.ActiveSelf)
                {
                    actor.Tick(deltaTime);
                }
            }
            _actors.RemoveAll(actor => actor.IsDestroyed);
        }

        protected override void OnDestroying()
        {
            for (int i = _actors.Count - 1; i >= 0; --i)
            {
                _actors[i].Destroy();
            }
            _actors.Clear();
            _hasBegunPlay = false;
        }
    }
}
