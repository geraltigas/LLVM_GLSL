; ModuleID = 'GLSL'
source_filename = "GLSL"

@gl_Position = external global <4 x float>
@FragColor = external global <4 x float>

define void @main() {
entry:
  store <4 x float> <float 1.000000e+00, float 5.000000e-01, float 0x3FC99999A0000000, float 1.000000e+00>, ptr @FragColor, align 16
  %0 = load <4 x float>, ptr @FragColor, align 16
}
