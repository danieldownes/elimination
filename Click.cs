using Godot;
using System;

public class Click : Spatial
{
    public override void _Ready()
    {
        
    }

    public void _on_Area_input_event(Camera camera, InputEventMouseButton evt, Vector3 click_pos, object click_normal, object shape_idx)
    {
        if( evt.Pressed)
            GD.Print(evt.ButtonMask);
    }
}
