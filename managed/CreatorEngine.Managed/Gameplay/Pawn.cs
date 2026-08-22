namespace CreatorEngine
{
    public class Pawn : Actor
    {
        public Pawn(string? name = null) : base(name)
        {
            InputComponent = AddComponent<EnhancedInputComponent>();
        }

        public Controller? Controller { get; private set; }
        public EnhancedInputComponent InputComponent { get; }

        public virtual void SetupPlayerInputComponent(EnhancedInputComponent input) { }

        internal void PossessedBy(Controller controller)
        {
            Controller = controller;
            SetupPlayerInputComponent(InputComponent);
            OnPossessed(controller);
        }

        internal void UnPossessed()
        {
            Controller? oldController = Controller;
            Controller = null;
            InputComponent.ClearBindings();
            if (oldController != null)
            {
                OnUnPossessed(oldController);
            }
        }

        protected virtual void OnPossessed(Controller controller) { }
        protected virtual void OnUnPossessed(Controller controller) { }

        public override void EndPlay()
        {
            Controller?.UnPossess();
            base.EndPlay();
        }
    }
}
