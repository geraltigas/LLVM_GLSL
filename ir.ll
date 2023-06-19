; ModuleID = 'GLSL'
source_filename = "GLSL"

@gl_Position = external global <4 x float>
@aPos = external global <3 x float>

define void @main() {
entry:
  %oldvalue = load <3 x float>, ptr @aPos, align 16
  %newvalue = extractelement <3 x float> %oldvalue, i64 0
  %oldvalue1 = load <3 x float>, ptr @aPos, align 16
  %newvalue2 = extractelement <3 x float> %oldvalue1, i64 0
  %0 = insertelement <4 x float> undef, float %newvalue2, i64 0
  %oldvalue3 = load <3 x float>, ptr @aPos, align 16
  %newvalue4 = extractelement <3 x float> %oldvalue3, i64 1
  %1 = insertelement <4 x float> %0, float %newvalue4, i64 1
  %oldvalue5 = load <3 x float>, ptr @aPos, align 16
  %newvalue6 = extractelement <3 x float> %oldvalue5, i64 2
  %2 = insertelement <4 x float> %1, float %newvalue6, i64 2
  %3 = insertelement <4 x float> %2, float 1.000000e+00, i64 3
  %4 = alloca <4 x float>, align 16
  store <4 x float> %3, ptr %4, align 16
  %vec = load <4 x float>, ptr %4, align 16
  store <4 x float> %vec, ptr @gl_Position, align 16
  ret void <badref>
}
