; ModuleID = 'main-extracted.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.timespec = type { i64, i64 }

@.str.2 = external hidden unnamed_addr constant [27 x i8], align 1
@.str.3 = external hidden unnamed_addr constant [2 x i8], align 1
@.str.4 = external hidden unnamed_addr constant [6 x i8], align 1
@.str.5 = external hidden unnamed_addr constant [23 x i8], align 1
@.str.6 = external hidden unnamed_addr constant [4 x i8], align 1

; Function Attrs: nounwind
declare noalias ptr @malloc(i64) #0

; Function Attrs: nounwind
declare i32 @clock_gettime(i32, ptr) #0

declare i32 @printf(ptr, ...) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main(i32 %0, ptr %1) #2 {
  %3 = alloca i64, align 8, !ValueID !5, !BlockID !6
  %4 = alloca ptr, align 8, !ValueID !7
  %5 = alloca ptr, align 8, !ValueID !8
  %6 = alloca ptr, align 8, !ValueID !9
  %7 = alloca ptr, align 8, !ValueID !10
  %8 = alloca i8, align 1, !ValueID !11
  %9 = alloca %struct.timespec, align 8, !ValueID !12
  %10 = alloca %struct.timespec, align 8, !ValueID !13
  %11 = alloca %struct.timespec, align 8, !ValueID !14
  %12 = alloca %struct.timespec, align 8, !ValueID !15
  %.reg2mem = alloca i32, align 4, !ValueID !16
  %.reg2mem8 = alloca i64, align 8, !ValueID !17
  %.reg2mem10 = alloca i64, align 8, !ValueID !18
  %.reg2mem12 = alloca i64, align 8, !ValueID !19
  %.reg2mem14 = alloca i32, align 4, !ValueID !20
  %.735.reg2mem = alloca i32, align 4, !ValueID !21
  %.722.reg2mem = alloca i64, align 8, !ValueID !22
  %.reg2mem32 = alloca ptr, align 8, !ValueID !23
  %.reg2mem36 = alloca ptr, align 8, !ValueID !24
  %.79.reg2mem = alloca i64, align 8, !ValueID !25
  %.7.reg2mem = alloca i64, align 8, !ValueID !26
  %.reg2mem45 = alloca ptr, align 8, !ValueID !27
  %.750.reg2mem = alloca i32, align 4, !ValueID !28
  %.reg2mem51 = alloca i32, align 4, !ValueID !29
  %.reg2mem53 = alloca i64, align 8, !ValueID !30
  %.reg2mem55 = alloca i64, align 8, !ValueID !31
  %.reg2mem57 = alloca i64, align 8, !ValueID !32
  %.reg2mem59 = alloca i32, align 4, !ValueID !33
  %.reg2mem61 = alloca ptr, align 8, !ValueID !34
  %.reg2mem63 = alloca ptr, align 8, !ValueID !35
  %.reg2mem66 = alloca ptr, align 8, !ValueID !36
  %.reg2mem69 = alloca ptr, align 8, !ValueID !37
  %.634.reg2mem = alloca i32, align 4, !ValueID !38
  %.621.reg2mem = alloca i64, align 8, !ValueID !39
  %.reg2mem89 = alloca ptr, align 8, !ValueID !40
  %.reg2mem93 = alloca ptr, align 8, !ValueID !41
  %.68.reg2mem = alloca i64, align 8, !ValueID !42
  %.6.reg2mem = alloca i64, align 8, !ValueID !43
  %.reg2mem102 = alloca ptr, align 8, !ValueID !44
  %.649.reg2mem = alloca i32, align 4, !ValueID !45
  %.reg2mem108 = alloca ptr, align 8, !ValueID !46
  %.reg2mem110 = alloca ptr, align 8, !ValueID !47
  %.reg2mem113 = alloca ptr, align 8, !ValueID !48
  %.reg2mem116 = alloca ptr, align 8, !ValueID !49
  %.reg2mem120 = alloca i32, align 4, !ValueID !50
  %.533.reg2mem = alloca i32, align 4, !ValueID !51
  %.reg2mem127 = alloca i64, align 8, !ValueID !52
  %.520.reg2mem = alloca i64, align 8, !ValueID !53
  %.reg2mem140 = alloca ptr, align 8, !ValueID !54
  %.reg2mem144 = alloca i64, align 8, !ValueID !55
  %.reg2mem146 = alloca ptr, align 8, !ValueID !56
  %.57.reg2mem = alloca i64, align 8, !ValueID !57
  %.reg2mem152 = alloca i64, align 8, !ValueID !58
  %.5.reg2mem = alloca i64, align 8, !ValueID !59
  %.reg2mem157 = alloca ptr, align 8, !ValueID !60
  %.reg2mem161 = alloca i32, align 4, !ValueID !61
  %.548.reg2mem = alloca i32, align 4, !ValueID !62
  %.reg2mem165 = alloca i32, align 4, !ValueID !63
  %.reg2mem167 = alloca i64, align 8, !ValueID !64
  %.reg2mem169 = alloca i64, align 8, !ValueID !65
  %.reg2mem171 = alloca i64, align 8, !ValueID !66
  %.reg2mem173 = alloca i32, align 4, !ValueID !67
  %.reg2mem175 = alloca ptr, align 8, !ValueID !68
  %.reg2mem177 = alloca ptr, align 8, !ValueID !69
  %.reg2mem180 = alloca ptr, align 8, !ValueID !70
  %.reg2mem183 = alloca ptr, align 8, !ValueID !71
  %.432.reg2mem = alloca i32, align 4, !ValueID !72
  %.419.reg2mem = alloca i64, align 8, !ValueID !73
  %.reg2mem203 = alloca ptr, align 8, !ValueID !74
  %.reg2mem207 = alloca ptr, align 8, !ValueID !75
  %.46.reg2mem = alloca i64, align 8, !ValueID !76
  %.4.reg2mem = alloca i64, align 8, !ValueID !77
  %.reg2mem216 = alloca ptr, align 8, !ValueID !78
  %.447.reg2mem = alloca i32, align 4, !ValueID !79
  %.reg2mem222 = alloca i32, align 4, !ValueID !80
  %.reg2mem224 = alloca i64, align 8, !ValueID !81
  %.reg2mem226 = alloca i64, align 8, !ValueID !82
  %.reg2mem228 = alloca i64, align 8, !ValueID !83
  %.reg2mem230 = alloca i32, align 4, !ValueID !84
  %.reg2mem232 = alloca ptr, align 8, !ValueID !85
  %.reg2mem234 = alloca ptr, align 8, !ValueID !86
  %.reg2mem237 = alloca ptr, align 8, !ValueID !87
  %.reg2mem240 = alloca ptr, align 8, !ValueID !88
  %.331.reg2mem = alloca i32, align 4, !ValueID !89
  %.318.reg2mem = alloca i64, align 8, !ValueID !90
  %.reg2mem260 = alloca ptr, align 8, !ValueID !91
  %.reg2mem264 = alloca ptr, align 8, !ValueID !92
  %.35.reg2mem = alloca i64, align 8, !ValueID !93
  %.3.reg2mem = alloca i64, align 8, !ValueID !94
  %.reg2mem273 = alloca ptr, align 8, !ValueID !95
  %.346.reg2mem = alloca i32, align 4, !ValueID !96
  %.reg2mem279 = alloca i32, align 4, !ValueID !97
  %.reg2mem281 = alloca i64, align 8, !ValueID !98
  %.reg2mem283 = alloca i64, align 8, !ValueID !99
  %.reg2mem285 = alloca i64, align 8, !ValueID !100
  %.reg2mem287 = alloca i32, align 4, !ValueID !101
  %.reg2mem289 = alloca ptr, align 8, !ValueID !102
  %.reg2mem291 = alloca ptr, align 8, !ValueID !103
  %.reg2mem294 = alloca ptr, align 8, !ValueID !104
  %.reg2mem297 = alloca ptr, align 8, !ValueID !105
  %.230.reg2mem = alloca i32, align 4, !ValueID !106
  %.217.reg2mem = alloca i64, align 8, !ValueID !107
  %.reg2mem317 = alloca ptr, align 8, !ValueID !108
  %.reg2mem321 = alloca ptr, align 8, !ValueID !109
  %.24.reg2mem = alloca i64, align 8, !ValueID !110
  %.2.reg2mem = alloca i64, align 8, !ValueID !111
  %.reg2mem330 = alloca ptr, align 8, !ValueID !112
  %.245.reg2mem = alloca i32, align 4, !ValueID !113
  %.reg2mem336 = alloca i32, align 4, !ValueID !114
  %.reg2mem338 = alloca i64, align 8, !ValueID !115
  %.reg2mem340 = alloca i64, align 8, !ValueID !116
  %.reg2mem342 = alloca i64, align 8, !ValueID !117
  %.reg2mem344 = alloca i32, align 4, !ValueID !118
  %.reg2mem346 = alloca ptr, align 8, !ValueID !119
  %.reg2mem348 = alloca ptr, align 8, !ValueID !120
  %.reg2mem351 = alloca ptr, align 8, !ValueID !121
  %.reg2mem354 = alloca ptr, align 8, !ValueID !122
  %.129.reg2mem = alloca i32, align 4, !ValueID !123
  %.116.reg2mem = alloca i64, align 8, !ValueID !124
  %.reg2mem374 = alloca ptr, align 8, !ValueID !125
  %.reg2mem378 = alloca ptr, align 8, !ValueID !126
  %.13.reg2mem = alloca i64, align 8, !ValueID !127
  %.1.reg2mem = alloca i64, align 8, !ValueID !128
  %.reg2mem387 = alloca ptr, align 8, !ValueID !129
  %.144.reg2mem = alloca i32, align 4, !ValueID !130
  %.reg2mem393 = alloca i32, align 4, !ValueID !131
  %.reg2mem395 = alloca i64, align 8, !ValueID !132
  %.reg2mem397 = alloca i64, align 8, !ValueID !133
  %.reg2mem399 = alloca i64, align 8, !ValueID !134
  %.reg2mem401 = alloca i32, align 4, !ValueID !135
  %.reg2mem403 = alloca ptr, align 8, !ValueID !136
  %.reg2mem405 = alloca ptr, align 8, !ValueID !137
  %.reg2mem408 = alloca ptr, align 8, !ValueID !138
  %.reg2mem411 = alloca ptr, align 8, !ValueID !139
  %.028.reg2mem = alloca i32, align 4, !ValueID !140
  %.015.reg2mem = alloca i64, align 8, !ValueID !141
  %.reg2mem431 = alloca ptr, align 8, !ValueID !142
  %.reg2mem435 = alloca ptr, align 8, !ValueID !143
  %.01.reg2mem = alloca i64, align 8, !ValueID !144
  %.0.reg2mem = alloca i64, align 8, !ValueID !145
  %.reg2mem444 = alloca ptr, align 8, !ValueID !146
  %.043.reg2mem = alloca i32, align 4, !ValueID !147
  %.reg2mem450 = alloca ptr, align 8, !ValueID !148
  %.reg2mem452 = alloca ptr, align 8, !ValueID !149
  %.reg2mem455 = alloca ptr, align 8, !ValueID !150
  %.reg2mem458 = alloca ptr, align 8, !ValueID !151
  %.reg2mem462 = alloca i32, align 4, !ValueID !152
  %.042.reg2mem = alloca i32, align 4, !ValueID !153
  %.reg2mem466 = alloca ptr, align 8, !ValueID !154
  %.reg2mem476 = alloca ptr, align 8, !ValueID !155
  %.735.reg2mem480 = alloca i32, align 4, !ValueID !156
  %.722.reg2mem482 = alloca i64, align 8, !ValueID !157
  %.79.reg2mem484 = alloca i64, align 8, !ValueID !158
  %.7.reg2mem486 = alloca i64, align 8, !ValueID !159
  %.750.reg2mem488 = alloca i32, align 4, !ValueID !160
  %.634.reg2mem490 = alloca i32, align 4, !ValueID !161
  %.621.reg2mem492 = alloca i64, align 8, !ValueID !162
  %.68.reg2mem494 = alloca i64, align 8, !ValueID !163
  %.6.reg2mem496 = alloca i64, align 8, !ValueID !164
  %.649.reg2mem498 = alloca i32, align 4, !ValueID !165
  %.533.reg2mem500 = alloca i32, align 4, !ValueID !166
  %.520.reg2mem502 = alloca i64, align 8, !ValueID !167
  %.57.reg2mem504 = alloca i64, align 8, !ValueID !168
  %.5.reg2mem506 = alloca i64, align 8, !ValueID !169
  %.548.reg2mem508 = alloca i32, align 4, !ValueID !170
  %.432.reg2mem510 = alloca i32, align 4, !ValueID !171
  %.419.reg2mem512 = alloca i64, align 8, !ValueID !172
  %.46.reg2mem514 = alloca i64, align 8, !ValueID !173
  %.4.reg2mem516 = alloca i64, align 8, !ValueID !174
  %.447.reg2mem518 = alloca i32, align 4, !ValueID !175
  %.331.reg2mem520 = alloca i32, align 4, !ValueID !176
  %.318.reg2mem522 = alloca i64, align 8, !ValueID !177
  %.35.reg2mem524 = alloca i64, align 8, !ValueID !178
  %.3.reg2mem526 = alloca i64, align 8, !ValueID !179
  %.346.reg2mem528 = alloca i32, align 4, !ValueID !180
  %.230.reg2mem530 = alloca i32, align 4, !ValueID !181
  %.217.reg2mem532 = alloca i64, align 8, !ValueID !182
  %.24.reg2mem534 = alloca i64, align 8, !ValueID !183
  %.2.reg2mem536 = alloca i64, align 8, !ValueID !184
  %.245.reg2mem538 = alloca i32, align 4, !ValueID !185
  %.129.reg2mem540 = alloca i32, align 4, !ValueID !186
  %.116.reg2mem542 = alloca i64, align 8, !ValueID !187
  %.13.reg2mem544 = alloca i64, align 8, !ValueID !188
  %.1.reg2mem546 = alloca i64, align 8, !ValueID !189
  %.144.reg2mem548 = alloca i32, align 4, !ValueID !190
  %.028.reg2mem550 = alloca i32, align 4, !ValueID !191
  %.015.reg2mem552 = alloca i64, align 8, !ValueID !192
  %.01.reg2mem554 = alloca i64, align 8, !ValueID !193
  %.0.reg2mem556 = alloca i64, align 8, !ValueID !194
  %.043.reg2mem558 = alloca i32, align 4, !ValueID !195
  %.042.reg2mem560 = alloca i32, align 4, !ValueID !196
  %"reg2mem alloca point" = bitcast i32 0 to i32, !ValueID !197
  %13 = call i32 @clock_gettime(i32 4, ptr %10) #5, !ValueID !198
  %14 = call noalias ptr @fopen(ptr @.str.2, ptr @.str.3), !ValueID !199
  %15 = bitcast ptr %.reg2mem476 to ptr, !ValueID !200
  store ptr %14, ptr %.reg2mem476, align 8, !ValueID !201
  %16 = bitcast ptr %.reg2mem476 to ptr, !ValueID !202
  %.reload479 = load ptr, ptr %.reg2mem476, align 8, !ValueID !203
  %17 = icmp eq ptr %.reload479, null, !ValueID !204
  br i1 %17, label %18, label %20, !ValueID !205

18:                                               ; preds = %2
  call void @perror(ptr @.str.4), !ValueID !206, !BlockID !207
  %19 = call i32 (ptr, ...) @printf(ptr @.str.5, ptr @.str.2), !ValueID !208
  call void @exit(i32 1) #6, !ValueID !209
  unreachable, !ValueID !210

20:                                               ; preds = %2
  %21 = call noalias ptr @malloc(i64 2048) #5, !ValueID !211, !BlockID !212
  %22 = bitcast ptr %21 to ptr, !ValueID !213
  %23 = bitcast ptr %.reg2mem466 to ptr, !ValueID !214
  store ptr %22, ptr %.reg2mem466, align 8, !ValueID !215
  %24 = bitcast ptr %.042.reg2mem560 to ptr, !ValueID !216
  store i32 0, ptr %.042.reg2mem560, align 4, !ValueID !217
  br label %25, !ValueID !218

25:                                               ; preds = %31, %20
  %26 = bitcast ptr %.042.reg2mem560 to ptr, !ValueID !219
  %.042.reload561 = load i32, ptr %.042.reg2mem560, align 4, !ValueID !220, !BlockID !221
  %27 = bitcast ptr %.042.reg2mem to ptr, !ValueID !222
  store i32 %.042.reload561, ptr %.042.reg2mem, align 4, !ValueID !223
  %28 = bitcast ptr %.042.reg2mem to ptr, !ValueID !224
  %.042.reload465 = load i32, ptr %.042.reg2mem, align 4, !ValueID !225
  %29 = sext i32 %.042.reload465 to i64, !ValueID !226
  %30 = icmp ult i64 %29, 256, !ValueID !227
  br i1 %30, label %31, label %.peel.begin, !ValueID !228

31:                                               ; preds = %25
  %32 = bitcast ptr %.042.reg2mem to ptr, !ValueID !229
  %.042.reload464 = load i32, ptr %.042.reg2mem, align 4, !ValueID !230, !BlockID !231
  %33 = sext i32 %.042.reload464 to i64, !ValueID !232
  %34 = bitcast ptr %.reg2mem466 to ptr, !ValueID !233
  %.reload475 = load ptr, ptr %.reg2mem466, align 8, !ValueID !234
  %35 = getelementptr inbounds double, ptr %.reload475, i64 %33, !ValueID !235
  %36 = bitcast ptr %.reg2mem476 to ptr, !ValueID !236
  %.reload478 = load ptr, ptr %.reg2mem476, align 8, !ValueID !237
  %37 = call i32 (ptr, ptr, ...) @__isoc99_fscanf(ptr %.reload478, ptr @.str.6, ptr %35), !ValueID !238
  %38 = bitcast ptr %.042.reg2mem to ptr, !ValueID !239
  %.042.reload = load i32, ptr %.042.reg2mem, align 4, !ValueID !240
  call void @ComputeDump(), !ValueID !241
  %39 = add nsw i32 %.042.reload, 1, !ValueID !242
  %40 = bitcast ptr %.reg2mem462 to ptr, !ValueID !243
  store i32 %39, ptr %.reg2mem462, align 4, !ValueID !244
  %41 = bitcast ptr %.reg2mem462 to ptr, !ValueID !245
  %.reload463 = load i32, ptr %.reg2mem462, align 4, !ValueID !246
  %42 = bitcast ptr %.042.reg2mem560 to ptr, !ValueID !247
  store i32 %.reload463, ptr %.042.reg2mem560, align 4, !ValueID !248
  br label %25, !ValueID !249

.peel.begin:                                      ; preds = %25
  %43 = bitcast ptr %.reg2mem476 to ptr, !ValueID !250
  %.reload477 = load ptr, ptr %.reg2mem476, align 8, !ValueID !251, !BlockID !252
  %44 = call i32 @fclose(ptr %.reload477), !ValueID !253
  %45 = call i32 @clock_gettime(i32 4, ptr %12) #5, !ValueID !254
  br label %46, !ValueID !255

46:                                               ; preds = %.peel.begin
  call void (...) @NonKernelSplit(), !ValueID !256, !BlockID !257
  br label %codeRepl, !ValueID !258

codeRepl:                                         ; preds = %46
  call void @llvm.lifetime.start.p0(i64 -1, ptr %3)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @_task_1(ptr %.reg2mem458, ptr %.reg2mem455, ptr %.reg2mem452, ptr %.reg2mem450, ptr %.043.reg2mem558, ptr %.043.reg2mem, ptr %3, ptr %4, ptr %5, ptr %6, ptr %7, ptr %8, ptr %9, ptr %.reg2mem444, ptr %.0.reg2mem556, ptr %.0.reg2mem, ptr %.01.reg2mem554, ptr %.01.reg2mem, ptr %.reg2mem435, ptr %.reg2mem466, ptr %.reg2mem431, ptr %.015.reg2mem552, ptr %.015.reg2mem, ptr %12, ptr %.028.reg2mem550, ptr %.028.reg2mem, ptr %.reg2mem401, ptr %.reg2mem399, ptr %.reg2mem397, ptr %.reg2mem395, ptr %.reg2mem393, ptr %.reg2mem69, ptr %.reg2mem66, ptr %.reg2mem63, ptr %.reg2mem61, ptr %.750.reg2mem488, ptr %.750.reg2mem, ptr %.reg2mem45, ptr %.7.reg2mem486, ptr %.7.reg2mem, ptr %.79.reg2mem484, ptr %.79.reg2mem, ptr %.reg2mem36, ptr %.reg2mem32, ptr %.722.reg2mem482, ptr %.722.reg2mem, ptr %.735.reg2mem480, ptr %.735.reg2mem, ptr %.reg2mem14, ptr %.reg2mem12, ptr %.reg2mem10, ptr %.reg2mem8, ptr %.reg2mem)
  br label %codeRepl1

codeRepl1:                                        ; preds = %codeRepl
  call void @llvm.lifetime.start.p0(i64 -1, ptr %3)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @_task_4(ptr %.reg2mem411, ptr %.reg2mem408, ptr %.reg2mem405, ptr %.reg2mem403, ptr %.144.reg2mem548, ptr %.144.reg2mem, ptr %3, ptr %4, ptr %5, ptr %6, ptr %7, ptr %8, ptr %9, ptr %.reg2mem387, ptr %.1.reg2mem546, ptr %.1.reg2mem, ptr %.13.reg2mem544, ptr %.13.reg2mem, ptr %.reg2mem378, ptr %.reg2mem466, ptr %.reg2mem374, ptr %.116.reg2mem542, ptr %.116.reg2mem, ptr %12, ptr %.129.reg2mem540, ptr %.129.reg2mem, ptr %.reg2mem344, ptr %.reg2mem342, ptr %.reg2mem340, ptr %.reg2mem338, ptr %.reg2mem336, ptr %.reg2mem116, ptr %.reg2mem113, ptr %.reg2mem110, ptr %.reg2mem108, ptr %.649.reg2mem498, ptr %.649.reg2mem, ptr %.reg2mem102, ptr %.6.reg2mem496, ptr %.6.reg2mem, ptr %.68.reg2mem494, ptr %.68.reg2mem, ptr %.reg2mem93, ptr %.reg2mem89, ptr %.621.reg2mem492, ptr %.621.reg2mem, ptr %.634.reg2mem490, ptr %.634.reg2mem, ptr %.reg2mem59, ptr %.reg2mem57, ptr %.reg2mem55, ptr %.reg2mem53, ptr %.reg2mem51)
  br label %codeRepl2

codeRepl2:                                        ; preds = %codeRepl1
  call void @llvm.lifetime.start.p0(i64 -1, ptr %3)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @_task_7(ptr %.reg2mem354, ptr %.reg2mem351, ptr %.reg2mem348, ptr %.reg2mem346, ptr %.245.reg2mem538, ptr %.245.reg2mem, ptr %3, ptr %4, ptr %5, ptr %6, ptr %7, ptr %8, ptr %9, ptr %.reg2mem330, ptr %.2.reg2mem536, ptr %.2.reg2mem, ptr %.24.reg2mem534, ptr %.24.reg2mem, ptr %.reg2mem321, ptr %.reg2mem466, ptr %.reg2mem317, ptr %.217.reg2mem532, ptr %.217.reg2mem, ptr %12, ptr %.230.reg2mem530, ptr %.230.reg2mem, ptr %.reg2mem287, ptr %.reg2mem285, ptr %.reg2mem283, ptr %.reg2mem281, ptr %.reg2mem279, ptr %.reg2mem183, ptr %.reg2mem180, ptr %.reg2mem177, ptr %.reg2mem175, ptr %.548.reg2mem508, ptr %.548.reg2mem, ptr %.reg2mem161, ptr %.reg2mem157, ptr %.5.reg2mem506, ptr %.5.reg2mem, ptr %.57.reg2mem504, ptr %.reg2mem152, ptr %.57.reg2mem, ptr %.reg2mem146, ptr %.reg2mem144, ptr %.reg2mem140, ptr %.520.reg2mem502, ptr %.520.reg2mem, ptr %.reg2mem127, ptr %.533.reg2mem500, ptr %.533.reg2mem, ptr %.reg2mem120)
  br label %codeRepl3

codeRepl3:                                        ; preds = %codeRepl2
  call void @llvm.lifetime.start.p0(i64 -1, ptr %3)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @_task_10(ptr %.reg2mem297, ptr %.reg2mem294, ptr %.reg2mem291, ptr %.reg2mem289, ptr %.346.reg2mem528, ptr %.346.reg2mem, ptr %3, ptr %4, ptr %5, ptr %6, ptr %7, ptr %8, ptr %9, ptr %.reg2mem273, ptr %.3.reg2mem526, ptr %.3.reg2mem, ptr %.35.reg2mem524, ptr %.35.reg2mem, ptr %.reg2mem264, ptr %.reg2mem466, ptr %.reg2mem260, ptr %.318.reg2mem522, ptr %.318.reg2mem, ptr %12, ptr %.331.reg2mem520, ptr %.331.reg2mem, ptr %.reg2mem240, ptr %.reg2mem237, ptr %.reg2mem234, ptr %.reg2mem232, ptr %.447.reg2mem518, ptr %.reg2mem230, ptr %.reg2mem228, ptr %.reg2mem226, ptr %.reg2mem224, ptr %.reg2mem222, ptr %.447.reg2mem, ptr %.reg2mem216, ptr %.4.reg2mem516, ptr %.4.reg2mem, ptr %.46.reg2mem514, ptr %.46.reg2mem, ptr %.reg2mem207, ptr %.reg2mem203, ptr %.419.reg2mem512, ptr %.419.reg2mem, ptr %.432.reg2mem510, ptr %.432.reg2mem, ptr %.reg2mem173, ptr %.reg2mem171, ptr %.reg2mem169, ptr %.reg2mem167, ptr %.reg2mem165)
  br label %codeRepl4

codeRepl4:                                        ; preds = %codeRepl3
  call void @_task_25(ptr %11, ptr %10)
  br label %.ret

.ret:                                             ; preds = %codeRepl4
  ret i32 0, !ValueID !259
}

declare noalias ptr @fopen(ptr, ptr) #1

declare void @perror(ptr) #1

; Function Attrs: noreturn nounwind
declare void @exit(i32) #3

declare i32 @__isoc99_fscanf(ptr, ptr, ...) #1

declare i32 @fclose(ptr) #1

declare void @NonKernelSplit(...) #1

declare void @ComputeDump()

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #4

; Function Attrs: noinline nounwind uwtable
declare hidden void @_task_1(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #2

; Function Attrs: noinline nounwind uwtable
declare hidden void @_task_4(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #2

; Function Attrs: noinline nounwind uwtable
declare hidden void @_task_7(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #2

; Function Attrs: noinline nounwind uwtable
declare hidden void @_task_10(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #2

; Function Attrs: noinline nounwind uwtable
declare hidden void @_task_25(ptr, ptr) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { nounwind }
attributes #6 = { noreturn nounwind }

!llvm.ident = !{!0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{!"clang version 9.0.1 "}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 0}
!5 = !{i64 832}
!6 = !{i64 28}
!7 = !{i64 833}
!8 = !{i64 834}
!9 = !{i64 835}
!10 = !{i64 836}
!11 = !{i64 837}
!12 = !{i64 838}
!13 = !{i64 839}
!14 = !{i64 840}
!15 = !{i64 841}
!16 = !{i64 842}
!17 = !{i64 843}
!18 = !{i64 844}
!19 = !{i64 845}
!20 = !{i64 846}
!21 = !{i64 847}
!22 = !{i64 848}
!23 = !{i64 849}
!24 = !{i64 850}
!25 = !{i64 851}
!26 = !{i64 852}
!27 = !{i64 853}
!28 = !{i64 854}
!29 = !{i64 855}
!30 = !{i64 856}
!31 = !{i64 857}
!32 = !{i64 858}
!33 = !{i64 859}
!34 = !{i64 860}
!35 = !{i64 861}
!36 = !{i64 862}
!37 = !{i64 863}
!38 = !{i64 864}
!39 = !{i64 865}
!40 = !{i64 866}
!41 = !{i64 867}
!42 = !{i64 868}
!43 = !{i64 869}
!44 = !{i64 870}
!45 = !{i64 871}
!46 = !{i64 872}
!47 = !{i64 873}
!48 = !{i64 874}
!49 = !{i64 875}
!50 = !{i64 876}
!51 = !{i64 877}
!52 = !{i64 878}
!53 = !{i64 879}
!54 = !{i64 880}
!55 = !{i64 881}
!56 = !{i64 882}
!57 = !{i64 883}
!58 = !{i64 884}
!59 = !{i64 885}
!60 = !{i64 886}
!61 = !{i64 887}
!62 = !{i64 888}
!63 = !{i64 889}
!64 = !{i64 890}
!65 = !{i64 891}
!66 = !{i64 892}
!67 = !{i64 893}
!68 = !{i64 894}
!69 = !{i64 895}
!70 = !{i64 896}
!71 = !{i64 897}
!72 = !{i64 898}
!73 = !{i64 899}
!74 = !{i64 900}
!75 = !{i64 901}
!76 = !{i64 902}
!77 = !{i64 903}
!78 = !{i64 904}
!79 = !{i64 905}
!80 = !{i64 906}
!81 = !{i64 907}
!82 = !{i64 908}
!83 = !{i64 909}
!84 = !{i64 910}
!85 = !{i64 911}
!86 = !{i64 912}
!87 = !{i64 913}
!88 = !{i64 914}
!89 = !{i64 915}
!90 = !{i64 916}
!91 = !{i64 917}
!92 = !{i64 918}
!93 = !{i64 919}
!94 = !{i64 920}
!95 = !{i64 921}
!96 = !{i64 922}
!97 = !{i64 923}
!98 = !{i64 924}
!99 = !{i64 925}
!100 = !{i64 926}
!101 = !{i64 927}
!102 = !{i64 928}
!103 = !{i64 929}
!104 = !{i64 930}
!105 = !{i64 931}
!106 = !{i64 932}
!107 = !{i64 933}
!108 = !{i64 934}
!109 = !{i64 935}
!110 = !{i64 936}
!111 = !{i64 937}
!112 = !{i64 938}
!113 = !{i64 939}
!114 = !{i64 940}
!115 = !{i64 941}
!116 = !{i64 942}
!117 = !{i64 943}
!118 = !{i64 944}
!119 = !{i64 945}
!120 = !{i64 946}
!121 = !{i64 947}
!122 = !{i64 948}
!123 = !{i64 949}
!124 = !{i64 950}
!125 = !{i64 951}
!126 = !{i64 952}
!127 = !{i64 953}
!128 = !{i64 954}
!129 = !{i64 955}
!130 = !{i64 956}
!131 = !{i64 957}
!132 = !{i64 958}
!133 = !{i64 959}
!134 = !{i64 960}
!135 = !{i64 961}
!136 = !{i64 962}
!137 = !{i64 963}
!138 = !{i64 964}
!139 = !{i64 965}
!140 = !{i64 966}
!141 = !{i64 967}
!142 = !{i64 968}
!143 = !{i64 969}
!144 = !{i64 970}
!145 = !{i64 971}
!146 = !{i64 972}
!147 = !{i64 973}
!148 = !{i64 974}
!149 = !{i64 975}
!150 = !{i64 976}
!151 = !{i64 977}
!152 = !{i64 978}
!153 = !{i64 979}
!154 = !{i64 980}
!155 = !{i64 981}
!156 = !{i64 982}
!157 = !{i64 983}
!158 = !{i64 984}
!159 = !{i64 985}
!160 = !{i64 986}
!161 = !{i64 987}
!162 = !{i64 988}
!163 = !{i64 989}
!164 = !{i64 990}
!165 = !{i64 991}
!166 = !{i64 992}
!167 = !{i64 993}
!168 = !{i64 994}
!169 = !{i64 995}
!170 = !{i64 996}
!171 = !{i64 997}
!172 = !{i64 998}
!173 = !{i64 999}
!174 = !{i64 1000}
!175 = !{i64 1001}
!176 = !{i64 1002}
!177 = !{i64 1003}
!178 = !{i64 1004}
!179 = !{i64 1005}
!180 = !{i64 1006}
!181 = !{i64 1007}
!182 = !{i64 1008}
!183 = !{i64 1009}
!184 = !{i64 1010}
!185 = !{i64 1011}
!186 = !{i64 1012}
!187 = !{i64 1013}
!188 = !{i64 1014}
!189 = !{i64 1015}
!190 = !{i64 1016}
!191 = !{i64 1017}
!192 = !{i64 1018}
!193 = !{i64 1019}
!194 = !{i64 1020}
!195 = !{i64 1021}
!196 = !{i64 1022}
!197 = !{i64 1023}
!198 = !{i64 1024}
!199 = !{i64 1025}
!200 = !{i64 1026}
!201 = !{i64 1028}
!202 = !{i64 1029}
!203 = !{i64 1031}
!204 = !{i64 1032}
!205 = !{i64 1034}
!206 = !{i64 1036}
!207 = !{i64 29}
!208 = !{i64 1037}
!209 = !{i64 1038}
!210 = !{i64 1040}
!211 = !{i64 1042}
!212 = !{i64 30}
!213 = !{i64 1043}
!214 = !{i64 1044}
!215 = !{i64 1046}
!216 = !{i64 1047}
!217 = !{i64 1049}
!218 = !{i64 1051}
!219 = !{i64 1053}
!220 = !{i64 1055}
!221 = !{i64 31}
!222 = !{i64 1056}
!223 = !{i64 1058}
!224 = !{i64 1059}
!225 = !{i64 1061}
!226 = !{i64 1062}
!227 = !{i64 1063}
!228 = !{i64 1065}
!229 = !{i64 1067}
!230 = !{i64 1069}
!231 = !{i64 32}
!232 = !{i64 1070}
!233 = !{i64 1071}
!234 = !{i64 1073}
!235 = !{i64 1074}
!236 = !{i64 1075}
!237 = !{i64 1077}
!238 = !{i64 1078}
!239 = !{i64 1079}
!240 = !{i64 1081}
!241 = !{i64 1082}
!242 = !{i64 1083}
!243 = !{i64 1084}
!244 = !{i64 1086}
!245 = !{i64 1087}
!246 = !{i64 1089}
!247 = !{i64 1090}
!248 = !{i64 1092}
!249 = !{i64 1094}
!250 = !{i64 1096}
!251 = !{i64 1098}
!252 = !{i64 33}
!253 = !{i64 1099}
!254 = !{i64 1100}
!255 = !{i64 1102}
!256 = !{i64 1104}
!257 = !{i64 34}
!258 = !{i64 1106}
!259 = !{i64 6165}
