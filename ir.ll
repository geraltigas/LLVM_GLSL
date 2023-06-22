; ModuleID = 'GLSL'
source_filename = "GLSL"

@gl_Position = external global <4 x float>

define i32 @get_int() {
entry:
  ret i32 1
}

define i32 @get_int_(i32 %a) {
entry:
  ret i32 %a
}

define <3 x float> @get_vec3() {
entry:
  ret void
}
