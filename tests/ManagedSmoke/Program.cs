using System;
using System.Linq;
using CreatorEngine;

internal static class Program
{
    private static int Main()
    {
        try
        {
            VerifyDefaultWorldAndInput();
            VerifyHierarchyCycleProtection();
            VerifyAnimationGraph();
            Console.WriteLine("CreatorEngine managed smoke tests passed.");
            return 0;
        }
        catch (Exception exception)
        {
            Console.Error.WriteLine(exception);
            return 1;
        }
    }

    private static void VerifyDefaultWorldAndInput()
    {
        World world = DefaultWorldBootstrap.CreatePlayableWorld();
        Require(world.GameMode is DefaultGameMode, "Default GameMode was not created.");
        Require(world.PrimaryPlayerController is DefaultPlayerController,
            "Default PlayerController was not created.");
        Require(world.DefaultPawn is DefaultPawn, "Default Pawn was not created.");

        var pawn = (DefaultPawn)world.DefaultPawn!;
        Vector3 before = pawn.Transform.Position;
        var pressed = new InputFrame()
            .SetButton("Keyboard.W", true)
            .SetButton("Keyboard.D", true)
            .SetButton("Keyboard.Space", true);

        world.PrimaryPlayerController!.ProcessInput(pressed, 1.0f / 60.0f);
        world.Tick(1.0f);
        Require((pawn.Transform.Position - before).Length > 4.9f,
            "Enhanced Input did not drive Character movement.");
        Require(pawn.IsJumping, "Pressed trigger did not invoke Jump.");

        world.PrimaryPlayerController.ProcessInput(new InputFrame(), 1.0f / 60.0f);
        Require(!pawn.IsJumping, "Completed event did not invoke StopJumping.");
        world.Destroy();
    }

    private static void VerifyHierarchyCycleProtection()
    {
        var parent = new GameObject("Parent");
        var child = new GameObject("Child");
        child.Transform.SetParent(parent.Transform);

        CameraComponent removable = parent.AddComponent<CameraComponent>();
        removable.Destroy();
        Require(parent.GetComponent<CameraComponent>() == null,
            "Destroyed Component remained attached to its GameObject.");

        bool rejected = false;
        try
        {
            parent.Transform.SetParent(child.Transform);
        }
        catch (InvalidOperationException)
        {
            rejected = true;
        }
        Require(rejected, "Transform accepted a cyclic hierarchy.");
        parent.Destroy();
    }

    private static void VerifyAnimationGraph()
    {
        var idleClip = new AnimationClip("Idle", 1.0f);
        var runClip = new AnimationClip("Run", 0.5f);
        var blend = new BlendSpace1DNode("Speed");
        blend.AddSample(0.0f, new AnimationClipNode(idleClip));
        blend.AddSample(6.0f, new AnimationClipNode(runClip));

        var stateMachine = new AnimationStateMachineNode("Locomotion");
        stateMachine.AddState(new AnimationState("Locomotion", blend));
        var graph = new AnimationGraph("DefaultLocomotion", stateMachine);
        var instance = new AnimationGraphInstance(graph);
        instance.SetSpeed(3.0f);

        AnimationPose pose = instance.Evaluate(1.0f / 60.0f);
        Require(pose.Clips.Count == 2, "Blend Space did not return two weighted clips.");
        Require(pose.Clips.Any(sample => sample.Clip == idleClip && sample.Weight > 0.0f),
            "Blend Space omitted the idle sample.");
        Require(pose.Clips.Any(sample => sample.Clip == runClip && sample.Weight > 0.0f),
            "Blend Space omitted the run sample.");

        var machine = new AnimationStateMachineNode("Idle");
        machine.AddState(new AnimationState("Idle", new AnimationClipNode(idleClip)));
        machine.AddState(new AnimationState("Run", new AnimationClipNode(runClip)));
        var transition = new AnimationTransition("Idle", "Run") { DurationSeconds = 0.01f };
        transition.Conditions.Add(new FloatAnimationCondition(
            "Speed", AnimationComparison.Greater, 0.1f));
        machine.AddTransition(transition);

        var stateGraph = new AnimationGraph("StateMachine", machine);
        var stateInstance = new AnimationGraphInstance(stateGraph);
        stateInstance.SetSpeed(2.0f);
        AnimationPose statePose = stateInstance.Evaluate(1.0f / 60.0f);
        Require(statePose.Clips.Any(sample => sample.Clip == runClip),
            "Animation State Machine did not enter the Run state.");
    }

    private static void Require(bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}
