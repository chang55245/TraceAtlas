; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

%struct.timespec = type { i64, i64 }
%TaskArgs.1 = type { ptr, i64 }
%TaskArg.1 = type { ptr, i1 }
%TaskArgs.4 = type { ptr, i64 }
%TaskArg.4 = type { ptr, i1 }
%TaskArgs.7 = type { ptr, i64 }
%TaskArg.7 = type { ptr, i1 }
%TaskArgs.10 = type { ptr, i64 }
%TaskArg.10 = type { ptr, i1 }
%TaskArgs.25 = type { ptr, i64 }
%TaskArg.25 = type { ptr, i1 }

@func_name_25 = internal constant [16 x i8] c"taskflow_task_25"
@func_name_10 = internal constant [16 x i8] c"taskflow_task_10"
@func_name_7 = internal constant [15 x i8] c"taskflow_task_7"
@func_name_4 = internal constant [15 x i8] c"taskflow_task_4"
@func_name_1 = internal constant [15 x i8] c"taskflow_task_1"
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
define i32 @main(i32 %0, ptr %1) #2 {
  %3 = call ptr @taskflow_create()
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i8, align 1
  %10 = alloca %struct.timespec, align 8
  %11 = alloca %struct.timespec, align 8
  %12 = alloca %struct.timespec, align 8
  %13 = alloca %struct.timespec, align 8
  %14 = alloca i32, align 4
  %15 = alloca i64, align 8
  %16 = alloca i64, align 8
  %17 = alloca i64, align 8
  %18 = alloca i32, align 4
  %19 = alloca i32, align 4
  %20 = alloca i64, align 8
  %21 = alloca ptr, align 8
  %22 = alloca ptr, align 8
  %23 = alloca i64, align 8
  %24 = alloca i64, align 8
  %25 = alloca ptr, align 8
  %26 = alloca i32, align 4
  %27 = alloca i32, align 4
  %28 = alloca i64, align 8
  %29 = alloca i64, align 8
  %30 = alloca i64, align 8
  %31 = alloca i32, align 4
  %32 = alloca ptr, align 8
  %33 = alloca ptr, align 8
  %34 = alloca ptr, align 8
  %35 = alloca ptr, align 8
  %36 = alloca i32, align 4
  %37 = alloca i64, align 8
  %38 = alloca ptr, align 8
  %39 = alloca ptr, align 8
  %40 = alloca i64, align 8
  %41 = alloca i64, align 8
  %42 = alloca ptr, align 8
  %43 = alloca i32, align 4
  %44 = alloca ptr, align 8
  %45 = alloca ptr, align 8
  %46 = alloca ptr, align 8
  %47 = alloca ptr, align 8
  %48 = alloca i32, align 4
  %49 = alloca i32, align 4
  %50 = alloca i64, align 8
  %51 = alloca i64, align 8
  %52 = alloca ptr, align 8
  %53 = alloca i64, align 8
  %54 = alloca ptr, align 8
  %55 = alloca i64, align 8
  %56 = alloca i64, align 8
  %57 = alloca i64, align 8
  %58 = alloca ptr, align 8
  %59 = alloca i32, align 4
  %60 = alloca i32, align 4
  %61 = alloca i32, align 4
  %62 = alloca i64, align 8
  %63 = alloca i64, align 8
  %64 = alloca i64, align 8
  %65 = alloca i32, align 4
  %66 = alloca ptr, align 8
  %67 = alloca ptr, align 8
  %68 = alloca ptr, align 8
  %69 = alloca ptr, align 8
  %70 = alloca i32, align 4
  %71 = alloca i64, align 8
  %72 = alloca ptr, align 8
  %73 = alloca ptr, align 8
  %74 = alloca i64, align 8
  %75 = alloca i64, align 8
  %76 = alloca ptr, align 8
  %77 = alloca i32, align 4
  %78 = alloca i32, align 4
  %79 = alloca i64, align 8
  %80 = alloca i64, align 8
  %81 = alloca i64, align 8
  %82 = alloca i32, align 4
  %83 = alloca ptr, align 8
  %84 = alloca ptr, align 8
  %85 = alloca ptr, align 8
  %86 = alloca ptr, align 8
  %87 = alloca i32, align 4
  %88 = alloca i64, align 8
  %89 = alloca ptr, align 8
  %90 = alloca ptr, align 8
  %91 = alloca i64, align 8
  %92 = alloca i64, align 8
  %93 = alloca ptr, align 8
  %94 = alloca i32, align 4
  %95 = alloca i32, align 4
  %96 = alloca i64, align 8
  %97 = alloca i64, align 8
  %98 = alloca i64, align 8
  %99 = alloca i32, align 4
  %100 = alloca ptr, align 8
  %101 = alloca ptr, align 8
  %102 = alloca ptr, align 8
  %103 = alloca ptr, align 8
  %104 = alloca i32, align 4
  %105 = alloca i64, align 8
  %106 = alloca ptr, align 8
  %107 = alloca ptr, align 8
  %108 = alloca i64, align 8
  %109 = alloca i64, align 8
  %110 = alloca ptr, align 8
  %111 = alloca i32, align 4
  %112 = alloca i32, align 4
  %113 = alloca i64, align 8
  %114 = alloca i64, align 8
  %115 = alloca i64, align 8
  %116 = alloca i32, align 4
  %117 = alloca ptr, align 8
  %118 = alloca ptr, align 8
  %119 = alloca ptr, align 8
  %120 = alloca ptr, align 8
  %121 = alloca i32, align 4
  %122 = alloca i64, align 8
  %123 = alloca ptr, align 8
  %124 = alloca ptr, align 8
  %125 = alloca i64, align 8
  %126 = alloca i64, align 8
  %127 = alloca ptr, align 8
  %128 = alloca i32, align 4
  %129 = alloca i32, align 4
  %130 = alloca i64, align 8
  %131 = alloca i64, align 8
  %132 = alloca i64, align 8
  %133 = alloca i32, align 4
  %134 = alloca ptr, align 8
  %135 = alloca ptr, align 8
  %136 = alloca ptr, align 8
  %137 = alloca ptr, align 8
  %138 = alloca i32, align 4
  %139 = alloca i64, align 8
  %140 = alloca ptr, align 8
  %141 = alloca ptr, align 8
  %142 = alloca i64, align 8
  %143 = alloca i64, align 8
  %144 = alloca ptr, align 8
  %145 = alloca i32, align 4
  %146 = alloca ptr, align 8
  %147 = alloca ptr, align 8
  %148 = alloca ptr, align 8
  %149 = alloca ptr, align 8
  %150 = alloca i32, align 4
  %151 = alloca i32, align 4
  %152 = alloca ptr, align 8
  %153 = alloca ptr, align 8
  %154 = alloca i32, align 4
  %155 = alloca i64, align 8
  %156 = alloca i64, align 8
  %157 = alloca i64, align 8
  %158 = alloca i32, align 4
  %159 = alloca i32, align 4
  %160 = alloca i64, align 8
  %161 = alloca i64, align 8
  %162 = alloca i64, align 8
  %163 = alloca i32, align 4
  %164 = alloca i32, align 4
  %165 = alloca i64, align 8
  %166 = alloca i64, align 8
  %167 = alloca i64, align 8
  %168 = alloca i32, align 4
  %169 = alloca i32, align 4
  %170 = alloca i64, align 8
  %171 = alloca i64, align 8
  %172 = alloca i64, align 8
  %173 = alloca i32, align 4
  %174 = alloca i32, align 4
  %175 = alloca i64, align 8
  %176 = alloca i64, align 8
  %177 = alloca i64, align 8
  %178 = alloca i32, align 4
  %179 = alloca i32, align 4
  %180 = alloca i64, align 8
  %181 = alloca i64, align 8
  %182 = alloca i64, align 8
  %183 = alloca i32, align 4
  %184 = alloca i32, align 4
  %185 = alloca i64, align 8
  %186 = alloca i64, align 8
  %187 = alloca i64, align 8
  %188 = alloca i32, align 4
  %189 = alloca i32, align 4
  %190 = alloca i64, align 8
  %191 = alloca i64, align 8
  %192 = alloca i64, align 8
  %193 = alloca i32, align 4
  %194 = alloca i32, align 4
  %195 = call i32 @clock_gettime(i32 4, ptr %11)
  %196 = call ptr @fopen(ptr @.str.2, ptr @.str.3)
  store ptr %196, ptr %153, align 8
  %197 = load ptr, ptr %153, align 8
  %198 = icmp eq ptr %197, null
  br i1 %198, label %199, label %201

199:                                              ; preds = %2
  call void @perror(ptr @.str.4)
  %200 = call i32 (ptr, ...) @printf(ptr @.str.5, ptr @.str.2)
  call void @exit(i32 1)
  unreachable

201:                                              ; preds = %2
  %202 = call ptr @malloc(i64 2048)
  store ptr %202, ptr %152, align 8
  store i32 0, ptr %194, align 4
  br label %203

203:                                              ; preds = %208, %201
  %204 = load i32, ptr %194, align 4
  store i32 %204, ptr %151, align 4
  %205 = load i32, ptr %151, align 4
  %206 = sext i32 %205 to i64
  %207 = icmp ult i64 %206, 256
  br i1 %207, label %208, label %218

208:                                              ; preds = %203
  %209 = load i32, ptr %151, align 4
  %210 = sext i32 %209 to i64
  %211 = load ptr, ptr %152, align 8
  %212 = getelementptr inbounds double, ptr %211, i64 %210
  %213 = load ptr, ptr %153, align 8
  %214 = call i32 (ptr, ptr, ...) @__isoc99_fscanf(ptr %213, ptr @.str.6, ptr %212)
  %215 = load i32, ptr %151, align 4
  call void @ComputeDump()
  %216 = add nsw i32 %215, 1
  store i32 %216, ptr %150, align 4
  %217 = load i32, ptr %150, align 4
  store i32 %217, ptr %194, align 4
  br label %203

218:                                              ; preds = %203
  %219 = load ptr, ptr %153, align 8
  %220 = call i32 @fclose(ptr %219)
  %221 = call i32 @clock_gettime(i32 4, ptr %13)
  br label %222

222:                                              ; preds = %218
  call void (...) @NonKernelSplit()
  br label %223

223:                                              ; preds = %222
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %10)
  %224 = call ptr @create_task_args(i64 53)
  call void @set_task_arg_ptr(ptr %224, i32 0, ptr %149)
  call void @set_task_arg_ptr(ptr %224, i32 1, ptr %148)
  call void @set_task_arg_ptr(ptr %224, i32 2, ptr %147)
  call void @set_task_arg_ptr(ptr %224, i32 3, ptr %146)
  call void @set_task_arg_ptr(ptr %224, i32 4, ptr %193)
  call void @set_task_arg_ptr(ptr %224, i32 5, ptr %145)
  call void @set_task_arg_ptr(ptr %224, i32 6, ptr %4)
  call void @set_task_arg_ptr(ptr %224, i32 7, ptr %5)
  call void @set_task_arg_ptr(ptr %224, i32 8, ptr %6)
  call void @set_task_arg_ptr(ptr %224, i32 9, ptr %7)
  call void @set_task_arg_ptr(ptr %224, i32 10, ptr %8)
  call void @set_task_arg_ptr(ptr %224, i32 11, ptr %9)
  call void @set_task_arg_ptr(ptr %224, i32 12, ptr %10)
  call void @set_task_arg_ptr(ptr %224, i32 13, ptr %144)
  call void @set_task_arg_ptr(ptr %224, i32 14, ptr %192)
  call void @set_task_arg_ptr(ptr %224, i32 15, ptr %143)
  call void @set_task_arg_ptr(ptr %224, i32 16, ptr %191)
  call void @set_task_arg_ptr(ptr %224, i32 17, ptr %142)
  call void @set_task_arg_ptr(ptr %224, i32 18, ptr %141)
  call void @set_task_arg_ptr(ptr %224, i32 19, ptr %152)
  call void @set_task_arg_ptr(ptr %224, i32 20, ptr %140)
  call void @set_task_arg_ptr(ptr %224, i32 21, ptr %190)
  call void @set_task_arg_ptr(ptr %224, i32 22, ptr %139)
  call void @set_task_arg_ptr(ptr %224, i32 23, ptr %13)
  call void @set_task_arg_ptr(ptr %224, i32 24, ptr %189)
  call void @set_task_arg_ptr(ptr %224, i32 25, ptr %138)
  call void @set_task_arg_ptr(ptr %224, i32 26, ptr %133)
  call void @set_task_arg_ptr(ptr %224, i32 27, ptr %132)
  call void @set_task_arg_ptr(ptr %224, i32 28, ptr %131)
  call void @set_task_arg_ptr(ptr %224, i32 29, ptr %130)
  call void @set_task_arg_ptr(ptr %224, i32 30, ptr %129)
  call void @set_task_arg_ptr(ptr %224, i32 31, ptr %35)
  call void @set_task_arg_ptr(ptr %224, i32 32, ptr %34)
  call void @set_task_arg_ptr(ptr %224, i32 33, ptr %33)
  call void @set_task_arg_ptr(ptr %224, i32 34, ptr %32)
  call void @set_task_arg_ptr(ptr %224, i32 35, ptr %158)
  call void @set_task_arg_ptr(ptr %224, i32 36, ptr %26)
  call void @set_task_arg_ptr(ptr %224, i32 37, ptr %25)
  call void @set_task_arg_ptr(ptr %224, i32 38, ptr %157)
  call void @set_task_arg_ptr(ptr %224, i32 39, ptr %24)
  call void @set_task_arg_ptr(ptr %224, i32 40, ptr %156)
  call void @set_task_arg_ptr(ptr %224, i32 41, ptr %23)
  call void @set_task_arg_ptr(ptr %224, i32 42, ptr %22)
  call void @set_task_arg_ptr(ptr %224, i32 43, ptr %21)
  call void @set_task_arg_ptr(ptr %224, i32 44, ptr %155)
  call void @set_task_arg_ptr(ptr %224, i32 45, ptr %20)
  call void @set_task_arg_ptr(ptr %224, i32 46, ptr %154)
  call void @set_task_arg_ptr(ptr %224, i32 47, ptr %19)
  call void @set_task_arg_ptr(ptr %224, i32 48, ptr %18)
  call void @set_task_arg_ptr(ptr %224, i32 49, ptr %17)
  call void @set_task_arg_ptr(ptr %224, i32 50, ptr %16)
  call void @set_task_arg_ptr(ptr %224, i32 51, ptr %15)
  call void @set_task_arg_ptr(ptr %224, i32 52, ptr %14)
  %225 = call ptr @taskflow_create_task(ptr %3, ptr @func_name_1, ptr @taskflow_task_wrapper_1, ptr %224)
  br label %226

226:                                              ; preds = %223
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %10)
  %227 = call ptr @create_task_args(i64 53)
  call void @set_task_arg_ptr(ptr %227, i32 0, ptr %137)
  call void @set_task_arg_ptr(ptr %227, i32 1, ptr %136)
  call void @set_task_arg_ptr(ptr %227, i32 2, ptr %135)
  call void @set_task_arg_ptr(ptr %227, i32 3, ptr %134)
  call void @set_task_arg_ptr(ptr %227, i32 4, ptr %188)
  call void @set_task_arg_ptr(ptr %227, i32 5, ptr %128)
  call void @set_task_arg_ptr(ptr %227, i32 6, ptr %4)
  call void @set_task_arg_ptr(ptr %227, i32 7, ptr %5)
  call void @set_task_arg_ptr(ptr %227, i32 8, ptr %6)
  call void @set_task_arg_ptr(ptr %227, i32 9, ptr %7)
  call void @set_task_arg_ptr(ptr %227, i32 10, ptr %8)
  call void @set_task_arg_ptr(ptr %227, i32 11, ptr %9)
  call void @set_task_arg_ptr(ptr %227, i32 12, ptr %10)
  call void @set_task_arg_ptr(ptr %227, i32 13, ptr %127)
  call void @set_task_arg_ptr(ptr %227, i32 14, ptr %187)
  call void @set_task_arg_ptr(ptr %227, i32 15, ptr %126)
  call void @set_task_arg_ptr(ptr %227, i32 16, ptr %186)
  call void @set_task_arg_ptr(ptr %227, i32 17, ptr %125)
  call void @set_task_arg_ptr(ptr %227, i32 18, ptr %124)
  call void @set_task_arg_ptr(ptr %227, i32 19, ptr %152)
  call void @set_task_arg_ptr(ptr %227, i32 20, ptr %123)
  call void @set_task_arg_ptr(ptr %227, i32 21, ptr %185)
  call void @set_task_arg_ptr(ptr %227, i32 22, ptr %122)
  call void @set_task_arg_ptr(ptr %227, i32 23, ptr %13)
  call void @set_task_arg_ptr(ptr %227, i32 24, ptr %184)
  call void @set_task_arg_ptr(ptr %227, i32 25, ptr %121)
  call void @set_task_arg_ptr(ptr %227, i32 26, ptr %116)
  call void @set_task_arg_ptr(ptr %227, i32 27, ptr %115)
  call void @set_task_arg_ptr(ptr %227, i32 28, ptr %114)
  call void @set_task_arg_ptr(ptr %227, i32 29, ptr %113)
  call void @set_task_arg_ptr(ptr %227, i32 30, ptr %112)
  call void @set_task_arg_ptr(ptr %227, i32 31, ptr %47)
  call void @set_task_arg_ptr(ptr %227, i32 32, ptr %46)
  call void @set_task_arg_ptr(ptr %227, i32 33, ptr %45)
  call void @set_task_arg_ptr(ptr %227, i32 34, ptr %44)
  call void @set_task_arg_ptr(ptr %227, i32 35, ptr %163)
  call void @set_task_arg_ptr(ptr %227, i32 36, ptr %43)
  call void @set_task_arg_ptr(ptr %227, i32 37, ptr %42)
  call void @set_task_arg_ptr(ptr %227, i32 38, ptr %162)
  call void @set_task_arg_ptr(ptr %227, i32 39, ptr %41)
  call void @set_task_arg_ptr(ptr %227, i32 40, ptr %161)
  call void @set_task_arg_ptr(ptr %227, i32 41, ptr %40)
  call void @set_task_arg_ptr(ptr %227, i32 42, ptr %39)
  call void @set_task_arg_ptr(ptr %227, i32 43, ptr %38)
  call void @set_task_arg_ptr(ptr %227, i32 44, ptr %160)
  call void @set_task_arg_ptr(ptr %227, i32 45, ptr %37)
  call void @set_task_arg_ptr(ptr %227, i32 46, ptr %159)
  call void @set_task_arg_ptr(ptr %227, i32 47, ptr %36)
  call void @set_task_arg_ptr(ptr %227, i32 48, ptr %31)
  call void @set_task_arg_ptr(ptr %227, i32 49, ptr %30)
  call void @set_task_arg_ptr(ptr %227, i32 50, ptr %29)
  call void @set_task_arg_ptr(ptr %227, i32 51, ptr %28)
  call void @set_task_arg_ptr(ptr %227, i32 52, ptr %27)
  %228 = call ptr @taskflow_create_task(ptr %3, ptr @func_name_4, ptr @taskflow_task_wrapper_4, ptr %227)
  br label %229

229:                                              ; preds = %226
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %10)
  %230 = call ptr @create_task_args(i64 53)
  call void @set_task_arg_ptr(ptr %230, i32 0, ptr %120)
  call void @set_task_arg_ptr(ptr %230, i32 1, ptr %119)
  call void @set_task_arg_ptr(ptr %230, i32 2, ptr %118)
  call void @set_task_arg_ptr(ptr %230, i32 3, ptr %117)
  call void @set_task_arg_ptr(ptr %230, i32 4, ptr %183)
  call void @set_task_arg_ptr(ptr %230, i32 5, ptr %111)
  call void @set_task_arg_ptr(ptr %230, i32 6, ptr %4)
  call void @set_task_arg_ptr(ptr %230, i32 7, ptr %5)
  call void @set_task_arg_ptr(ptr %230, i32 8, ptr %6)
  call void @set_task_arg_ptr(ptr %230, i32 9, ptr %7)
  call void @set_task_arg_ptr(ptr %230, i32 10, ptr %8)
  call void @set_task_arg_ptr(ptr %230, i32 11, ptr %9)
  call void @set_task_arg_ptr(ptr %230, i32 12, ptr %10)
  call void @set_task_arg_ptr(ptr %230, i32 13, ptr %110)
  call void @set_task_arg_ptr(ptr %230, i32 14, ptr %182)
  call void @set_task_arg_ptr(ptr %230, i32 15, ptr %109)
  call void @set_task_arg_ptr(ptr %230, i32 16, ptr %181)
  call void @set_task_arg_ptr(ptr %230, i32 17, ptr %108)
  call void @set_task_arg_ptr(ptr %230, i32 18, ptr %107)
  call void @set_task_arg_ptr(ptr %230, i32 19, ptr %152)
  call void @set_task_arg_ptr(ptr %230, i32 20, ptr %106)
  call void @set_task_arg_ptr(ptr %230, i32 21, ptr %180)
  call void @set_task_arg_ptr(ptr %230, i32 22, ptr %105)
  call void @set_task_arg_ptr(ptr %230, i32 23, ptr %13)
  call void @set_task_arg_ptr(ptr %230, i32 24, ptr %179)
  call void @set_task_arg_ptr(ptr %230, i32 25, ptr %104)
  call void @set_task_arg_ptr(ptr %230, i32 26, ptr %99)
  call void @set_task_arg_ptr(ptr %230, i32 27, ptr %98)
  call void @set_task_arg_ptr(ptr %230, i32 28, ptr %97)
  call void @set_task_arg_ptr(ptr %230, i32 29, ptr %96)
  call void @set_task_arg_ptr(ptr %230, i32 30, ptr %95)
  call void @set_task_arg_ptr(ptr %230, i32 31, ptr %69)
  call void @set_task_arg_ptr(ptr %230, i32 32, ptr %68)
  call void @set_task_arg_ptr(ptr %230, i32 33, ptr %67)
  call void @set_task_arg_ptr(ptr %230, i32 34, ptr %66)
  call void @set_task_arg_ptr(ptr %230, i32 35, ptr %168)
  call void @set_task_arg_ptr(ptr %230, i32 36, ptr %60)
  call void @set_task_arg_ptr(ptr %230, i32 37, ptr %59)
  call void @set_task_arg_ptr(ptr %230, i32 38, ptr %58)
  call void @set_task_arg_ptr(ptr %230, i32 39, ptr %167)
  call void @set_task_arg_ptr(ptr %230, i32 40, ptr %57)
  call void @set_task_arg_ptr(ptr %230, i32 41, ptr %166)
  call void @set_task_arg_ptr(ptr %230, i32 42, ptr %56)
  call void @set_task_arg_ptr(ptr %230, i32 43, ptr %55)
  call void @set_task_arg_ptr(ptr %230, i32 44, ptr %54)
  call void @set_task_arg_ptr(ptr %230, i32 45, ptr %53)
  call void @set_task_arg_ptr(ptr %230, i32 46, ptr %52)
  call void @set_task_arg_ptr(ptr %230, i32 47, ptr %165)
  call void @set_task_arg_ptr(ptr %230, i32 48, ptr %51)
  call void @set_task_arg_ptr(ptr %230, i32 49, ptr %50)
  call void @set_task_arg_ptr(ptr %230, i32 50, ptr %164)
  call void @set_task_arg_ptr(ptr %230, i32 51, ptr %49)
  call void @set_task_arg_ptr(ptr %230, i32 52, ptr %48)
  %231 = call ptr @taskflow_create_task(ptr %3, ptr @func_name_7, ptr @taskflow_task_wrapper_7, ptr %230)
  br label %232

232:                                              ; preds = %229
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %10)
  %233 = call ptr @create_task_args(i64 53)
  call void @set_task_arg_ptr(ptr %233, i32 0, ptr %103)
  call void @set_task_arg_ptr(ptr %233, i32 1, ptr %102)
  call void @set_task_arg_ptr(ptr %233, i32 2, ptr %101)
  call void @set_task_arg_ptr(ptr %233, i32 3, ptr %100)
  call void @set_task_arg_ptr(ptr %233, i32 4, ptr %178)
  call void @set_task_arg_ptr(ptr %233, i32 5, ptr %94)
  call void @set_task_arg_ptr(ptr %233, i32 6, ptr %4)
  call void @set_task_arg_ptr(ptr %233, i32 7, ptr %5)
  call void @set_task_arg_ptr(ptr %233, i32 8, ptr %6)
  call void @set_task_arg_ptr(ptr %233, i32 9, ptr %7)
  call void @set_task_arg_ptr(ptr %233, i32 10, ptr %8)
  call void @set_task_arg_ptr(ptr %233, i32 11, ptr %9)
  call void @set_task_arg_ptr(ptr %233, i32 12, ptr %10)
  call void @set_task_arg_ptr(ptr %233, i32 13, ptr %93)
  call void @set_task_arg_ptr(ptr %233, i32 14, ptr %177)
  call void @set_task_arg_ptr(ptr %233, i32 15, ptr %92)
  call void @set_task_arg_ptr(ptr %233, i32 16, ptr %176)
  call void @set_task_arg_ptr(ptr %233, i32 17, ptr %91)
  call void @set_task_arg_ptr(ptr %233, i32 18, ptr %90)
  call void @set_task_arg_ptr(ptr %233, i32 19, ptr %152)
  call void @set_task_arg_ptr(ptr %233, i32 20, ptr %89)
  call void @set_task_arg_ptr(ptr %233, i32 21, ptr %175)
  call void @set_task_arg_ptr(ptr %233, i32 22, ptr %88)
  call void @set_task_arg_ptr(ptr %233, i32 23, ptr %13)
  call void @set_task_arg_ptr(ptr %233, i32 24, ptr %174)
  call void @set_task_arg_ptr(ptr %233, i32 25, ptr %87)
  call void @set_task_arg_ptr(ptr %233, i32 26, ptr %86)
  call void @set_task_arg_ptr(ptr %233, i32 27, ptr %85)
  call void @set_task_arg_ptr(ptr %233, i32 28, ptr %84)
  call void @set_task_arg_ptr(ptr %233, i32 29, ptr %83)
  call void @set_task_arg_ptr(ptr %233, i32 30, ptr %173)
  call void @set_task_arg_ptr(ptr %233, i32 31, ptr %82)
  call void @set_task_arg_ptr(ptr %233, i32 32, ptr %81)
  call void @set_task_arg_ptr(ptr %233, i32 33, ptr %80)
  call void @set_task_arg_ptr(ptr %233, i32 34, ptr %79)
  call void @set_task_arg_ptr(ptr %233, i32 35, ptr %78)
  call void @set_task_arg_ptr(ptr %233, i32 36, ptr %77)
  call void @set_task_arg_ptr(ptr %233, i32 37, ptr %76)
  call void @set_task_arg_ptr(ptr %233, i32 38, ptr %172)
  call void @set_task_arg_ptr(ptr %233, i32 39, ptr %75)
  call void @set_task_arg_ptr(ptr %233, i32 40, ptr %171)
  call void @set_task_arg_ptr(ptr %233, i32 41, ptr %74)
  call void @set_task_arg_ptr(ptr %233, i32 42, ptr %73)
  call void @set_task_arg_ptr(ptr %233, i32 43, ptr %72)
  call void @set_task_arg_ptr(ptr %233, i32 44, ptr %170)
  call void @set_task_arg_ptr(ptr %233, i32 45, ptr %71)
  call void @set_task_arg_ptr(ptr %233, i32 46, ptr %169)
  call void @set_task_arg_ptr(ptr %233, i32 47, ptr %70)
  call void @set_task_arg_ptr(ptr %233, i32 48, ptr %65)
  call void @set_task_arg_ptr(ptr %233, i32 49, ptr %64)
  call void @set_task_arg_ptr(ptr %233, i32 50, ptr %63)
  call void @set_task_arg_ptr(ptr %233, i32 51, ptr %62)
  call void @set_task_arg_ptr(ptr %233, i32 52, ptr %61)
  %234 = call ptr @taskflow_create_task(ptr %3, ptr @func_name_10, ptr @taskflow_task_wrapper_10, ptr %233)
  br label %235

235:                                              ; preds = %232
  %236 = call ptr @create_task_args(i64 2)
  call void @set_task_arg_ptr(ptr %236, i32 0, ptr %12)
  call void @set_task_arg_ptr(ptr %236, i32 1, ptr %11)
  %237 = call ptr @taskflow_create_task(ptr %3, ptr @func_name_25, ptr @taskflow_task_wrapper_25, ptr %236)
  call void @taskflow_add_dependency(ptr %225, ptr %228)
  call void @taskflow_add_dependency(ptr %228, ptr %231)
  call void @taskflow_add_dependency(ptr %231, ptr %234)
  call void @taskflow_add_dependency(ptr %234, ptr %237)
  call void @taskflow_execute(ptr %3)
  br label %238

238:                                              ; preds = %235
  ret i32 0
}

declare noalias ptr @fopen(ptr, ptr) #1

declare void @perror(ptr) #1

; Function Attrs: noreturn nounwind
declare void @exit(i32) #3

declare i32 @__isoc99_fscanf(ptr, ptr, ...) #1

declare i32 @fclose(ptr) #1

declare void @NonKernelSplit(...) #1

declare void @ComputeDump()

; Function Attrs: noinline nounwind uwtable
declare hidden void @taskflow_task_1(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #4

; Function Attrs: noinline nounwind uwtable
declare hidden void @taskflow_task_4(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #4

; Function Attrs: noinline nounwind uwtable
declare hidden void @taskflow_task_7(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #4

; Function Attrs: noinline nounwind uwtable
declare hidden void @taskflow_task_10(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr) #4

; Function Attrs: noinline nounwind uwtable
declare hidden void @taskflow_task_25(ptr, ptr) #4

declare ptr @taskflow_create()

declare ptr @taskflow_create_task(ptr, ptr, ptr, ptr)

declare void @taskflow_add_dependency(ptr, ptr)

declare void @taskflow_execute(ptr)

declare ptr @create_task_args(i64)

declare void @set_task_arg_ptr(ptr, i32, ptr)

define internal void @taskflow_task_wrapper_1(ptr %0) {
  %2 = getelementptr %TaskArgs.1, ptr %0, i32 0, i32 0
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr %TaskArg.1, ptr %3, i64 0
  %5 = getelementptr %TaskArg.1, ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr %TaskArg.1, ptr %3, i64 1
  %8 = getelementptr %TaskArg.1, ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr %TaskArg.1, ptr %3, i64 2
  %11 = getelementptr %TaskArg.1, ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = getelementptr %TaskArg.1, ptr %3, i64 3
  %14 = getelementptr %TaskArg.1, ptr %13, i32 0, i32 0
  %15 = load ptr, ptr %14, align 8
  %16 = getelementptr %TaskArg.1, ptr %3, i64 4
  %17 = getelementptr %TaskArg.1, ptr %16, i32 0, i32 0
  %18 = load ptr, ptr %17, align 8
  %19 = getelementptr %TaskArg.1, ptr %3, i64 5
  %20 = getelementptr %TaskArg.1, ptr %19, i32 0, i32 0
  %21 = load ptr, ptr %20, align 8
  %22 = getelementptr %TaskArg.1, ptr %3, i64 6
  %23 = getelementptr %TaskArg.1, ptr %22, i32 0, i32 0
  %24 = load ptr, ptr %23, align 8
  %25 = getelementptr %TaskArg.1, ptr %3, i64 7
  %26 = getelementptr %TaskArg.1, ptr %25, i32 0, i32 0
  %27 = load ptr, ptr %26, align 8
  %28 = getelementptr %TaskArg.1, ptr %3, i64 8
  %29 = getelementptr %TaskArg.1, ptr %28, i32 0, i32 0
  %30 = load ptr, ptr %29, align 8
  %31 = getelementptr %TaskArg.1, ptr %3, i64 9
  %32 = getelementptr %TaskArg.1, ptr %31, i32 0, i32 0
  %33 = load ptr, ptr %32, align 8
  %34 = getelementptr %TaskArg.1, ptr %3, i64 10
  %35 = getelementptr %TaskArg.1, ptr %34, i32 0, i32 0
  %36 = load ptr, ptr %35, align 8
  %37 = getelementptr %TaskArg.1, ptr %3, i64 11
  %38 = getelementptr %TaskArg.1, ptr %37, i32 0, i32 0
  %39 = load ptr, ptr %38, align 8
  %40 = getelementptr %TaskArg.1, ptr %3, i64 12
  %41 = getelementptr %TaskArg.1, ptr %40, i32 0, i32 0
  %42 = load ptr, ptr %41, align 8
  %43 = getelementptr %TaskArg.1, ptr %3, i64 13
  %44 = getelementptr %TaskArg.1, ptr %43, i32 0, i32 0
  %45 = load ptr, ptr %44, align 8
  %46 = getelementptr %TaskArg.1, ptr %3, i64 14
  %47 = getelementptr %TaskArg.1, ptr %46, i32 0, i32 0
  %48 = load ptr, ptr %47, align 8
  %49 = getelementptr %TaskArg.1, ptr %3, i64 15
  %50 = getelementptr %TaskArg.1, ptr %49, i32 0, i32 0
  %51 = load ptr, ptr %50, align 8
  %52 = getelementptr %TaskArg.1, ptr %3, i64 16
  %53 = getelementptr %TaskArg.1, ptr %52, i32 0, i32 0
  %54 = load ptr, ptr %53, align 8
  %55 = getelementptr %TaskArg.1, ptr %3, i64 17
  %56 = getelementptr %TaskArg.1, ptr %55, i32 0, i32 0
  %57 = load ptr, ptr %56, align 8
  %58 = getelementptr %TaskArg.1, ptr %3, i64 18
  %59 = getelementptr %TaskArg.1, ptr %58, i32 0, i32 0
  %60 = load ptr, ptr %59, align 8
  %61 = getelementptr %TaskArg.1, ptr %3, i64 19
  %62 = getelementptr %TaskArg.1, ptr %61, i32 0, i32 0
  %63 = load ptr, ptr %62, align 8
  %64 = getelementptr %TaskArg.1, ptr %3, i64 20
  %65 = getelementptr %TaskArg.1, ptr %64, i32 0, i32 0
  %66 = load ptr, ptr %65, align 8
  %67 = getelementptr %TaskArg.1, ptr %3, i64 21
  %68 = getelementptr %TaskArg.1, ptr %67, i32 0, i32 0
  %69 = load ptr, ptr %68, align 8
  %70 = getelementptr %TaskArg.1, ptr %3, i64 22
  %71 = getelementptr %TaskArg.1, ptr %70, i32 0, i32 0
  %72 = load ptr, ptr %71, align 8
  %73 = getelementptr %TaskArg.1, ptr %3, i64 23
  %74 = getelementptr %TaskArg.1, ptr %73, i32 0, i32 0
  %75 = load ptr, ptr %74, align 8
  %76 = getelementptr %TaskArg.1, ptr %3, i64 24
  %77 = getelementptr %TaskArg.1, ptr %76, i32 0, i32 0
  %78 = load ptr, ptr %77, align 8
  %79 = getelementptr %TaskArg.1, ptr %3, i64 25
  %80 = getelementptr %TaskArg.1, ptr %79, i32 0, i32 0
  %81 = load ptr, ptr %80, align 8
  %82 = getelementptr %TaskArg.1, ptr %3, i64 26
  %83 = getelementptr %TaskArg.1, ptr %82, i32 0, i32 0
  %84 = load ptr, ptr %83, align 8
  %85 = getelementptr %TaskArg.1, ptr %3, i64 27
  %86 = getelementptr %TaskArg.1, ptr %85, i32 0, i32 0
  %87 = load ptr, ptr %86, align 8
  %88 = getelementptr %TaskArg.1, ptr %3, i64 28
  %89 = getelementptr %TaskArg.1, ptr %88, i32 0, i32 0
  %90 = load ptr, ptr %89, align 8
  %91 = getelementptr %TaskArg.1, ptr %3, i64 29
  %92 = getelementptr %TaskArg.1, ptr %91, i32 0, i32 0
  %93 = load ptr, ptr %92, align 8
  %94 = getelementptr %TaskArg.1, ptr %3, i64 30
  %95 = getelementptr %TaskArg.1, ptr %94, i32 0, i32 0
  %96 = load ptr, ptr %95, align 8
  %97 = getelementptr %TaskArg.1, ptr %3, i64 31
  %98 = getelementptr %TaskArg.1, ptr %97, i32 0, i32 0
  %99 = load ptr, ptr %98, align 8
  %100 = getelementptr %TaskArg.1, ptr %3, i64 32
  %101 = getelementptr %TaskArg.1, ptr %100, i32 0, i32 0
  %102 = load ptr, ptr %101, align 8
  %103 = getelementptr %TaskArg.1, ptr %3, i64 33
  %104 = getelementptr %TaskArg.1, ptr %103, i32 0, i32 0
  %105 = load ptr, ptr %104, align 8
  %106 = getelementptr %TaskArg.1, ptr %3, i64 34
  %107 = getelementptr %TaskArg.1, ptr %106, i32 0, i32 0
  %108 = load ptr, ptr %107, align 8
  %109 = getelementptr %TaskArg.1, ptr %3, i64 35
  %110 = getelementptr %TaskArg.1, ptr %109, i32 0, i32 0
  %111 = load ptr, ptr %110, align 8
  %112 = getelementptr %TaskArg.1, ptr %3, i64 36
  %113 = getelementptr %TaskArg.1, ptr %112, i32 0, i32 0
  %114 = load ptr, ptr %113, align 8
  %115 = getelementptr %TaskArg.1, ptr %3, i64 37
  %116 = getelementptr %TaskArg.1, ptr %115, i32 0, i32 0
  %117 = load ptr, ptr %116, align 8
  %118 = getelementptr %TaskArg.1, ptr %3, i64 38
  %119 = getelementptr %TaskArg.1, ptr %118, i32 0, i32 0
  %120 = load ptr, ptr %119, align 8
  %121 = getelementptr %TaskArg.1, ptr %3, i64 39
  %122 = getelementptr %TaskArg.1, ptr %121, i32 0, i32 0
  %123 = load ptr, ptr %122, align 8
  %124 = getelementptr %TaskArg.1, ptr %3, i64 40
  %125 = getelementptr %TaskArg.1, ptr %124, i32 0, i32 0
  %126 = load ptr, ptr %125, align 8
  %127 = getelementptr %TaskArg.1, ptr %3, i64 41
  %128 = getelementptr %TaskArg.1, ptr %127, i32 0, i32 0
  %129 = load ptr, ptr %128, align 8
  %130 = getelementptr %TaskArg.1, ptr %3, i64 42
  %131 = getelementptr %TaskArg.1, ptr %130, i32 0, i32 0
  %132 = load ptr, ptr %131, align 8
  %133 = getelementptr %TaskArg.1, ptr %3, i64 43
  %134 = getelementptr %TaskArg.1, ptr %133, i32 0, i32 0
  %135 = load ptr, ptr %134, align 8
  %136 = getelementptr %TaskArg.1, ptr %3, i64 44
  %137 = getelementptr %TaskArg.1, ptr %136, i32 0, i32 0
  %138 = load ptr, ptr %137, align 8
  %139 = getelementptr %TaskArg.1, ptr %3, i64 45
  %140 = getelementptr %TaskArg.1, ptr %139, i32 0, i32 0
  %141 = load ptr, ptr %140, align 8
  %142 = getelementptr %TaskArg.1, ptr %3, i64 46
  %143 = getelementptr %TaskArg.1, ptr %142, i32 0, i32 0
  %144 = load ptr, ptr %143, align 8
  %145 = getelementptr %TaskArg.1, ptr %3, i64 47
  %146 = getelementptr %TaskArg.1, ptr %145, i32 0, i32 0
  %147 = load ptr, ptr %146, align 8
  %148 = getelementptr %TaskArg.1, ptr %3, i64 48
  %149 = getelementptr %TaskArg.1, ptr %148, i32 0, i32 0
  %150 = load ptr, ptr %149, align 8
  %151 = getelementptr %TaskArg.1, ptr %3, i64 49
  %152 = getelementptr %TaskArg.1, ptr %151, i32 0, i32 0
  %153 = load ptr, ptr %152, align 8
  %154 = getelementptr %TaskArg.1, ptr %3, i64 50
  %155 = getelementptr %TaskArg.1, ptr %154, i32 0, i32 0
  %156 = load ptr, ptr %155, align 8
  %157 = getelementptr %TaskArg.1, ptr %3, i64 51
  %158 = getelementptr %TaskArg.1, ptr %157, i32 0, i32 0
  %159 = load ptr, ptr %158, align 8
  %160 = getelementptr %TaskArg.1, ptr %3, i64 52
  %161 = getelementptr %TaskArg.1, ptr %160, i32 0, i32 0
  %162 = load ptr, ptr %161, align 8
  call void @taskflow_task_1(ptr %6, ptr %9, ptr %12, ptr %15, ptr %18, ptr %21, ptr %24, ptr %27, ptr %30, ptr %33, ptr %36, ptr %39, ptr %42, ptr %45, ptr %48, ptr %51, ptr %54, ptr %57, ptr %60, ptr %63, ptr %66, ptr %69, ptr %72, ptr %75, ptr %78, ptr %81, ptr %84, ptr %87, ptr %90, ptr %93, ptr %96, ptr %99, ptr %102, ptr %105, ptr %108, ptr %111, ptr %114, ptr %117, ptr %120, ptr %123, ptr %126, ptr %129, ptr %132, ptr %135, ptr %138, ptr %141, ptr %144, ptr %147, ptr %150, ptr %153, ptr %156, ptr %159, ptr %162)
  ret void
}

define internal void @taskflow_task_wrapper_4(ptr %0) {
  %2 = getelementptr %TaskArgs.4, ptr %0, i32 0, i32 0
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr %TaskArg.4, ptr %3, i64 0
  %5 = getelementptr %TaskArg.4, ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr %TaskArg.4, ptr %3, i64 1
  %8 = getelementptr %TaskArg.4, ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr %TaskArg.4, ptr %3, i64 2
  %11 = getelementptr %TaskArg.4, ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = getelementptr %TaskArg.4, ptr %3, i64 3
  %14 = getelementptr %TaskArg.4, ptr %13, i32 0, i32 0
  %15 = load ptr, ptr %14, align 8
  %16 = getelementptr %TaskArg.4, ptr %3, i64 4
  %17 = getelementptr %TaskArg.4, ptr %16, i32 0, i32 0
  %18 = load ptr, ptr %17, align 8
  %19 = getelementptr %TaskArg.4, ptr %3, i64 5
  %20 = getelementptr %TaskArg.4, ptr %19, i32 0, i32 0
  %21 = load ptr, ptr %20, align 8
  %22 = getelementptr %TaskArg.4, ptr %3, i64 6
  %23 = getelementptr %TaskArg.4, ptr %22, i32 0, i32 0
  %24 = load ptr, ptr %23, align 8
  %25 = getelementptr %TaskArg.4, ptr %3, i64 7
  %26 = getelementptr %TaskArg.4, ptr %25, i32 0, i32 0
  %27 = load ptr, ptr %26, align 8
  %28 = getelementptr %TaskArg.4, ptr %3, i64 8
  %29 = getelementptr %TaskArg.4, ptr %28, i32 0, i32 0
  %30 = load ptr, ptr %29, align 8
  %31 = getelementptr %TaskArg.4, ptr %3, i64 9
  %32 = getelementptr %TaskArg.4, ptr %31, i32 0, i32 0
  %33 = load ptr, ptr %32, align 8
  %34 = getelementptr %TaskArg.4, ptr %3, i64 10
  %35 = getelementptr %TaskArg.4, ptr %34, i32 0, i32 0
  %36 = load ptr, ptr %35, align 8
  %37 = getelementptr %TaskArg.4, ptr %3, i64 11
  %38 = getelementptr %TaskArg.4, ptr %37, i32 0, i32 0
  %39 = load ptr, ptr %38, align 8
  %40 = getelementptr %TaskArg.4, ptr %3, i64 12
  %41 = getelementptr %TaskArg.4, ptr %40, i32 0, i32 0
  %42 = load ptr, ptr %41, align 8
  %43 = getelementptr %TaskArg.4, ptr %3, i64 13
  %44 = getelementptr %TaskArg.4, ptr %43, i32 0, i32 0
  %45 = load ptr, ptr %44, align 8
  %46 = getelementptr %TaskArg.4, ptr %3, i64 14
  %47 = getelementptr %TaskArg.4, ptr %46, i32 0, i32 0
  %48 = load ptr, ptr %47, align 8
  %49 = getelementptr %TaskArg.4, ptr %3, i64 15
  %50 = getelementptr %TaskArg.4, ptr %49, i32 0, i32 0
  %51 = load ptr, ptr %50, align 8
  %52 = getelementptr %TaskArg.4, ptr %3, i64 16
  %53 = getelementptr %TaskArg.4, ptr %52, i32 0, i32 0
  %54 = load ptr, ptr %53, align 8
  %55 = getelementptr %TaskArg.4, ptr %3, i64 17
  %56 = getelementptr %TaskArg.4, ptr %55, i32 0, i32 0
  %57 = load ptr, ptr %56, align 8
  %58 = getelementptr %TaskArg.4, ptr %3, i64 18
  %59 = getelementptr %TaskArg.4, ptr %58, i32 0, i32 0
  %60 = load ptr, ptr %59, align 8
  %61 = getelementptr %TaskArg.4, ptr %3, i64 19
  %62 = getelementptr %TaskArg.4, ptr %61, i32 0, i32 0
  %63 = load ptr, ptr %62, align 8
  %64 = getelementptr %TaskArg.4, ptr %3, i64 20
  %65 = getelementptr %TaskArg.4, ptr %64, i32 0, i32 0
  %66 = load ptr, ptr %65, align 8
  %67 = getelementptr %TaskArg.4, ptr %3, i64 21
  %68 = getelementptr %TaskArg.4, ptr %67, i32 0, i32 0
  %69 = load ptr, ptr %68, align 8
  %70 = getelementptr %TaskArg.4, ptr %3, i64 22
  %71 = getelementptr %TaskArg.4, ptr %70, i32 0, i32 0
  %72 = load ptr, ptr %71, align 8
  %73 = getelementptr %TaskArg.4, ptr %3, i64 23
  %74 = getelementptr %TaskArg.4, ptr %73, i32 0, i32 0
  %75 = load ptr, ptr %74, align 8
  %76 = getelementptr %TaskArg.4, ptr %3, i64 24
  %77 = getelementptr %TaskArg.4, ptr %76, i32 0, i32 0
  %78 = load ptr, ptr %77, align 8
  %79 = getelementptr %TaskArg.4, ptr %3, i64 25
  %80 = getelementptr %TaskArg.4, ptr %79, i32 0, i32 0
  %81 = load ptr, ptr %80, align 8
  %82 = getelementptr %TaskArg.4, ptr %3, i64 26
  %83 = getelementptr %TaskArg.4, ptr %82, i32 0, i32 0
  %84 = load ptr, ptr %83, align 8
  %85 = getelementptr %TaskArg.4, ptr %3, i64 27
  %86 = getelementptr %TaskArg.4, ptr %85, i32 0, i32 0
  %87 = load ptr, ptr %86, align 8
  %88 = getelementptr %TaskArg.4, ptr %3, i64 28
  %89 = getelementptr %TaskArg.4, ptr %88, i32 0, i32 0
  %90 = load ptr, ptr %89, align 8
  %91 = getelementptr %TaskArg.4, ptr %3, i64 29
  %92 = getelementptr %TaskArg.4, ptr %91, i32 0, i32 0
  %93 = load ptr, ptr %92, align 8
  %94 = getelementptr %TaskArg.4, ptr %3, i64 30
  %95 = getelementptr %TaskArg.4, ptr %94, i32 0, i32 0
  %96 = load ptr, ptr %95, align 8
  %97 = getelementptr %TaskArg.4, ptr %3, i64 31
  %98 = getelementptr %TaskArg.4, ptr %97, i32 0, i32 0
  %99 = load ptr, ptr %98, align 8
  %100 = getelementptr %TaskArg.4, ptr %3, i64 32
  %101 = getelementptr %TaskArg.4, ptr %100, i32 0, i32 0
  %102 = load ptr, ptr %101, align 8
  %103 = getelementptr %TaskArg.4, ptr %3, i64 33
  %104 = getelementptr %TaskArg.4, ptr %103, i32 0, i32 0
  %105 = load ptr, ptr %104, align 8
  %106 = getelementptr %TaskArg.4, ptr %3, i64 34
  %107 = getelementptr %TaskArg.4, ptr %106, i32 0, i32 0
  %108 = load ptr, ptr %107, align 8
  %109 = getelementptr %TaskArg.4, ptr %3, i64 35
  %110 = getelementptr %TaskArg.4, ptr %109, i32 0, i32 0
  %111 = load ptr, ptr %110, align 8
  %112 = getelementptr %TaskArg.4, ptr %3, i64 36
  %113 = getelementptr %TaskArg.4, ptr %112, i32 0, i32 0
  %114 = load ptr, ptr %113, align 8
  %115 = getelementptr %TaskArg.4, ptr %3, i64 37
  %116 = getelementptr %TaskArg.4, ptr %115, i32 0, i32 0
  %117 = load ptr, ptr %116, align 8
  %118 = getelementptr %TaskArg.4, ptr %3, i64 38
  %119 = getelementptr %TaskArg.4, ptr %118, i32 0, i32 0
  %120 = load ptr, ptr %119, align 8
  %121 = getelementptr %TaskArg.4, ptr %3, i64 39
  %122 = getelementptr %TaskArg.4, ptr %121, i32 0, i32 0
  %123 = load ptr, ptr %122, align 8
  %124 = getelementptr %TaskArg.4, ptr %3, i64 40
  %125 = getelementptr %TaskArg.4, ptr %124, i32 0, i32 0
  %126 = load ptr, ptr %125, align 8
  %127 = getelementptr %TaskArg.4, ptr %3, i64 41
  %128 = getelementptr %TaskArg.4, ptr %127, i32 0, i32 0
  %129 = load ptr, ptr %128, align 8
  %130 = getelementptr %TaskArg.4, ptr %3, i64 42
  %131 = getelementptr %TaskArg.4, ptr %130, i32 0, i32 0
  %132 = load ptr, ptr %131, align 8
  %133 = getelementptr %TaskArg.4, ptr %3, i64 43
  %134 = getelementptr %TaskArg.4, ptr %133, i32 0, i32 0
  %135 = load ptr, ptr %134, align 8
  %136 = getelementptr %TaskArg.4, ptr %3, i64 44
  %137 = getelementptr %TaskArg.4, ptr %136, i32 0, i32 0
  %138 = load ptr, ptr %137, align 8
  %139 = getelementptr %TaskArg.4, ptr %3, i64 45
  %140 = getelementptr %TaskArg.4, ptr %139, i32 0, i32 0
  %141 = load ptr, ptr %140, align 8
  %142 = getelementptr %TaskArg.4, ptr %3, i64 46
  %143 = getelementptr %TaskArg.4, ptr %142, i32 0, i32 0
  %144 = load ptr, ptr %143, align 8
  %145 = getelementptr %TaskArg.4, ptr %3, i64 47
  %146 = getelementptr %TaskArg.4, ptr %145, i32 0, i32 0
  %147 = load ptr, ptr %146, align 8
  %148 = getelementptr %TaskArg.4, ptr %3, i64 48
  %149 = getelementptr %TaskArg.4, ptr %148, i32 0, i32 0
  %150 = load ptr, ptr %149, align 8
  %151 = getelementptr %TaskArg.4, ptr %3, i64 49
  %152 = getelementptr %TaskArg.4, ptr %151, i32 0, i32 0
  %153 = load ptr, ptr %152, align 8
  %154 = getelementptr %TaskArg.4, ptr %3, i64 50
  %155 = getelementptr %TaskArg.4, ptr %154, i32 0, i32 0
  %156 = load ptr, ptr %155, align 8
  %157 = getelementptr %TaskArg.4, ptr %3, i64 51
  %158 = getelementptr %TaskArg.4, ptr %157, i32 0, i32 0
  %159 = load ptr, ptr %158, align 8
  %160 = getelementptr %TaskArg.4, ptr %3, i64 52
  %161 = getelementptr %TaskArg.4, ptr %160, i32 0, i32 0
  %162 = load ptr, ptr %161, align 8
  call void @taskflow_task_4(ptr %6, ptr %9, ptr %12, ptr %15, ptr %18, ptr %21, ptr %24, ptr %27, ptr %30, ptr %33, ptr %36, ptr %39, ptr %42, ptr %45, ptr %48, ptr %51, ptr %54, ptr %57, ptr %60, ptr %63, ptr %66, ptr %69, ptr %72, ptr %75, ptr %78, ptr %81, ptr %84, ptr %87, ptr %90, ptr %93, ptr %96, ptr %99, ptr %102, ptr %105, ptr %108, ptr %111, ptr %114, ptr %117, ptr %120, ptr %123, ptr %126, ptr %129, ptr %132, ptr %135, ptr %138, ptr %141, ptr %144, ptr %147, ptr %150, ptr %153, ptr %156, ptr %159, ptr %162)
  ret void
}

define internal void @taskflow_task_wrapper_7(ptr %0) {
  %2 = getelementptr %TaskArgs.7, ptr %0, i32 0, i32 0
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr %TaskArg.7, ptr %3, i64 0
  %5 = getelementptr %TaskArg.7, ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr %TaskArg.7, ptr %3, i64 1
  %8 = getelementptr %TaskArg.7, ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr %TaskArg.7, ptr %3, i64 2
  %11 = getelementptr %TaskArg.7, ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = getelementptr %TaskArg.7, ptr %3, i64 3
  %14 = getelementptr %TaskArg.7, ptr %13, i32 0, i32 0
  %15 = load ptr, ptr %14, align 8
  %16 = getelementptr %TaskArg.7, ptr %3, i64 4
  %17 = getelementptr %TaskArg.7, ptr %16, i32 0, i32 0
  %18 = load ptr, ptr %17, align 8
  %19 = getelementptr %TaskArg.7, ptr %3, i64 5
  %20 = getelementptr %TaskArg.7, ptr %19, i32 0, i32 0
  %21 = load ptr, ptr %20, align 8
  %22 = getelementptr %TaskArg.7, ptr %3, i64 6
  %23 = getelementptr %TaskArg.7, ptr %22, i32 0, i32 0
  %24 = load ptr, ptr %23, align 8
  %25 = getelementptr %TaskArg.7, ptr %3, i64 7
  %26 = getelementptr %TaskArg.7, ptr %25, i32 0, i32 0
  %27 = load ptr, ptr %26, align 8
  %28 = getelementptr %TaskArg.7, ptr %3, i64 8
  %29 = getelementptr %TaskArg.7, ptr %28, i32 0, i32 0
  %30 = load ptr, ptr %29, align 8
  %31 = getelementptr %TaskArg.7, ptr %3, i64 9
  %32 = getelementptr %TaskArg.7, ptr %31, i32 0, i32 0
  %33 = load ptr, ptr %32, align 8
  %34 = getelementptr %TaskArg.7, ptr %3, i64 10
  %35 = getelementptr %TaskArg.7, ptr %34, i32 0, i32 0
  %36 = load ptr, ptr %35, align 8
  %37 = getelementptr %TaskArg.7, ptr %3, i64 11
  %38 = getelementptr %TaskArg.7, ptr %37, i32 0, i32 0
  %39 = load ptr, ptr %38, align 8
  %40 = getelementptr %TaskArg.7, ptr %3, i64 12
  %41 = getelementptr %TaskArg.7, ptr %40, i32 0, i32 0
  %42 = load ptr, ptr %41, align 8
  %43 = getelementptr %TaskArg.7, ptr %3, i64 13
  %44 = getelementptr %TaskArg.7, ptr %43, i32 0, i32 0
  %45 = load ptr, ptr %44, align 8
  %46 = getelementptr %TaskArg.7, ptr %3, i64 14
  %47 = getelementptr %TaskArg.7, ptr %46, i32 0, i32 0
  %48 = load ptr, ptr %47, align 8
  %49 = getelementptr %TaskArg.7, ptr %3, i64 15
  %50 = getelementptr %TaskArg.7, ptr %49, i32 0, i32 0
  %51 = load ptr, ptr %50, align 8
  %52 = getelementptr %TaskArg.7, ptr %3, i64 16
  %53 = getelementptr %TaskArg.7, ptr %52, i32 0, i32 0
  %54 = load ptr, ptr %53, align 8
  %55 = getelementptr %TaskArg.7, ptr %3, i64 17
  %56 = getelementptr %TaskArg.7, ptr %55, i32 0, i32 0
  %57 = load ptr, ptr %56, align 8
  %58 = getelementptr %TaskArg.7, ptr %3, i64 18
  %59 = getelementptr %TaskArg.7, ptr %58, i32 0, i32 0
  %60 = load ptr, ptr %59, align 8
  %61 = getelementptr %TaskArg.7, ptr %3, i64 19
  %62 = getelementptr %TaskArg.7, ptr %61, i32 0, i32 0
  %63 = load ptr, ptr %62, align 8
  %64 = getelementptr %TaskArg.7, ptr %3, i64 20
  %65 = getelementptr %TaskArg.7, ptr %64, i32 0, i32 0
  %66 = load ptr, ptr %65, align 8
  %67 = getelementptr %TaskArg.7, ptr %3, i64 21
  %68 = getelementptr %TaskArg.7, ptr %67, i32 0, i32 0
  %69 = load ptr, ptr %68, align 8
  %70 = getelementptr %TaskArg.7, ptr %3, i64 22
  %71 = getelementptr %TaskArg.7, ptr %70, i32 0, i32 0
  %72 = load ptr, ptr %71, align 8
  %73 = getelementptr %TaskArg.7, ptr %3, i64 23
  %74 = getelementptr %TaskArg.7, ptr %73, i32 0, i32 0
  %75 = load ptr, ptr %74, align 8
  %76 = getelementptr %TaskArg.7, ptr %3, i64 24
  %77 = getelementptr %TaskArg.7, ptr %76, i32 0, i32 0
  %78 = load ptr, ptr %77, align 8
  %79 = getelementptr %TaskArg.7, ptr %3, i64 25
  %80 = getelementptr %TaskArg.7, ptr %79, i32 0, i32 0
  %81 = load ptr, ptr %80, align 8
  %82 = getelementptr %TaskArg.7, ptr %3, i64 26
  %83 = getelementptr %TaskArg.7, ptr %82, i32 0, i32 0
  %84 = load ptr, ptr %83, align 8
  %85 = getelementptr %TaskArg.7, ptr %3, i64 27
  %86 = getelementptr %TaskArg.7, ptr %85, i32 0, i32 0
  %87 = load ptr, ptr %86, align 8
  %88 = getelementptr %TaskArg.7, ptr %3, i64 28
  %89 = getelementptr %TaskArg.7, ptr %88, i32 0, i32 0
  %90 = load ptr, ptr %89, align 8
  %91 = getelementptr %TaskArg.7, ptr %3, i64 29
  %92 = getelementptr %TaskArg.7, ptr %91, i32 0, i32 0
  %93 = load ptr, ptr %92, align 8
  %94 = getelementptr %TaskArg.7, ptr %3, i64 30
  %95 = getelementptr %TaskArg.7, ptr %94, i32 0, i32 0
  %96 = load ptr, ptr %95, align 8
  %97 = getelementptr %TaskArg.7, ptr %3, i64 31
  %98 = getelementptr %TaskArg.7, ptr %97, i32 0, i32 0
  %99 = load ptr, ptr %98, align 8
  %100 = getelementptr %TaskArg.7, ptr %3, i64 32
  %101 = getelementptr %TaskArg.7, ptr %100, i32 0, i32 0
  %102 = load ptr, ptr %101, align 8
  %103 = getelementptr %TaskArg.7, ptr %3, i64 33
  %104 = getelementptr %TaskArg.7, ptr %103, i32 0, i32 0
  %105 = load ptr, ptr %104, align 8
  %106 = getelementptr %TaskArg.7, ptr %3, i64 34
  %107 = getelementptr %TaskArg.7, ptr %106, i32 0, i32 0
  %108 = load ptr, ptr %107, align 8
  %109 = getelementptr %TaskArg.7, ptr %3, i64 35
  %110 = getelementptr %TaskArg.7, ptr %109, i32 0, i32 0
  %111 = load ptr, ptr %110, align 8
  %112 = getelementptr %TaskArg.7, ptr %3, i64 36
  %113 = getelementptr %TaskArg.7, ptr %112, i32 0, i32 0
  %114 = load ptr, ptr %113, align 8
  %115 = getelementptr %TaskArg.7, ptr %3, i64 37
  %116 = getelementptr %TaskArg.7, ptr %115, i32 0, i32 0
  %117 = load ptr, ptr %116, align 8
  %118 = getelementptr %TaskArg.7, ptr %3, i64 38
  %119 = getelementptr %TaskArg.7, ptr %118, i32 0, i32 0
  %120 = load ptr, ptr %119, align 8
  %121 = getelementptr %TaskArg.7, ptr %3, i64 39
  %122 = getelementptr %TaskArg.7, ptr %121, i32 0, i32 0
  %123 = load ptr, ptr %122, align 8
  %124 = getelementptr %TaskArg.7, ptr %3, i64 40
  %125 = getelementptr %TaskArg.7, ptr %124, i32 0, i32 0
  %126 = load ptr, ptr %125, align 8
  %127 = getelementptr %TaskArg.7, ptr %3, i64 41
  %128 = getelementptr %TaskArg.7, ptr %127, i32 0, i32 0
  %129 = load ptr, ptr %128, align 8
  %130 = getelementptr %TaskArg.7, ptr %3, i64 42
  %131 = getelementptr %TaskArg.7, ptr %130, i32 0, i32 0
  %132 = load ptr, ptr %131, align 8
  %133 = getelementptr %TaskArg.7, ptr %3, i64 43
  %134 = getelementptr %TaskArg.7, ptr %133, i32 0, i32 0
  %135 = load ptr, ptr %134, align 8
  %136 = getelementptr %TaskArg.7, ptr %3, i64 44
  %137 = getelementptr %TaskArg.7, ptr %136, i32 0, i32 0
  %138 = load ptr, ptr %137, align 8
  %139 = getelementptr %TaskArg.7, ptr %3, i64 45
  %140 = getelementptr %TaskArg.7, ptr %139, i32 0, i32 0
  %141 = load ptr, ptr %140, align 8
  %142 = getelementptr %TaskArg.7, ptr %3, i64 46
  %143 = getelementptr %TaskArg.7, ptr %142, i32 0, i32 0
  %144 = load ptr, ptr %143, align 8
  %145 = getelementptr %TaskArg.7, ptr %3, i64 47
  %146 = getelementptr %TaskArg.7, ptr %145, i32 0, i32 0
  %147 = load ptr, ptr %146, align 8
  %148 = getelementptr %TaskArg.7, ptr %3, i64 48
  %149 = getelementptr %TaskArg.7, ptr %148, i32 0, i32 0
  %150 = load ptr, ptr %149, align 8
  %151 = getelementptr %TaskArg.7, ptr %3, i64 49
  %152 = getelementptr %TaskArg.7, ptr %151, i32 0, i32 0
  %153 = load ptr, ptr %152, align 8
  %154 = getelementptr %TaskArg.7, ptr %3, i64 50
  %155 = getelementptr %TaskArg.7, ptr %154, i32 0, i32 0
  %156 = load ptr, ptr %155, align 8
  %157 = getelementptr %TaskArg.7, ptr %3, i64 51
  %158 = getelementptr %TaskArg.7, ptr %157, i32 0, i32 0
  %159 = load ptr, ptr %158, align 8
  %160 = getelementptr %TaskArg.7, ptr %3, i64 52
  %161 = getelementptr %TaskArg.7, ptr %160, i32 0, i32 0
  %162 = load ptr, ptr %161, align 8
  call void @taskflow_task_7(ptr %6, ptr %9, ptr %12, ptr %15, ptr %18, ptr %21, ptr %24, ptr %27, ptr %30, ptr %33, ptr %36, ptr %39, ptr %42, ptr %45, ptr %48, ptr %51, ptr %54, ptr %57, ptr %60, ptr %63, ptr %66, ptr %69, ptr %72, ptr %75, ptr %78, ptr %81, ptr %84, ptr %87, ptr %90, ptr %93, ptr %96, ptr %99, ptr %102, ptr %105, ptr %108, ptr %111, ptr %114, ptr %117, ptr %120, ptr %123, ptr %126, ptr %129, ptr %132, ptr %135, ptr %138, ptr %141, ptr %144, ptr %147, ptr %150, ptr %153, ptr %156, ptr %159, ptr %162)
  ret void
}

define internal void @taskflow_task_wrapper_10(ptr %0) {
  %2 = getelementptr %TaskArgs.10, ptr %0, i32 0, i32 0
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr %TaskArg.10, ptr %3, i64 0
  %5 = getelementptr %TaskArg.10, ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr %TaskArg.10, ptr %3, i64 1
  %8 = getelementptr %TaskArg.10, ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr %TaskArg.10, ptr %3, i64 2
  %11 = getelementptr %TaskArg.10, ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = getelementptr %TaskArg.10, ptr %3, i64 3
  %14 = getelementptr %TaskArg.10, ptr %13, i32 0, i32 0
  %15 = load ptr, ptr %14, align 8
  %16 = getelementptr %TaskArg.10, ptr %3, i64 4
  %17 = getelementptr %TaskArg.10, ptr %16, i32 0, i32 0
  %18 = load ptr, ptr %17, align 8
  %19 = getelementptr %TaskArg.10, ptr %3, i64 5
  %20 = getelementptr %TaskArg.10, ptr %19, i32 0, i32 0
  %21 = load ptr, ptr %20, align 8
  %22 = getelementptr %TaskArg.10, ptr %3, i64 6
  %23 = getelementptr %TaskArg.10, ptr %22, i32 0, i32 0
  %24 = load ptr, ptr %23, align 8
  %25 = getelementptr %TaskArg.10, ptr %3, i64 7
  %26 = getelementptr %TaskArg.10, ptr %25, i32 0, i32 0
  %27 = load ptr, ptr %26, align 8
  %28 = getelementptr %TaskArg.10, ptr %3, i64 8
  %29 = getelementptr %TaskArg.10, ptr %28, i32 0, i32 0
  %30 = load ptr, ptr %29, align 8
  %31 = getelementptr %TaskArg.10, ptr %3, i64 9
  %32 = getelementptr %TaskArg.10, ptr %31, i32 0, i32 0
  %33 = load ptr, ptr %32, align 8
  %34 = getelementptr %TaskArg.10, ptr %3, i64 10
  %35 = getelementptr %TaskArg.10, ptr %34, i32 0, i32 0
  %36 = load ptr, ptr %35, align 8
  %37 = getelementptr %TaskArg.10, ptr %3, i64 11
  %38 = getelementptr %TaskArg.10, ptr %37, i32 0, i32 0
  %39 = load ptr, ptr %38, align 8
  %40 = getelementptr %TaskArg.10, ptr %3, i64 12
  %41 = getelementptr %TaskArg.10, ptr %40, i32 0, i32 0
  %42 = load ptr, ptr %41, align 8
  %43 = getelementptr %TaskArg.10, ptr %3, i64 13
  %44 = getelementptr %TaskArg.10, ptr %43, i32 0, i32 0
  %45 = load ptr, ptr %44, align 8
  %46 = getelementptr %TaskArg.10, ptr %3, i64 14
  %47 = getelementptr %TaskArg.10, ptr %46, i32 0, i32 0
  %48 = load ptr, ptr %47, align 8
  %49 = getelementptr %TaskArg.10, ptr %3, i64 15
  %50 = getelementptr %TaskArg.10, ptr %49, i32 0, i32 0
  %51 = load ptr, ptr %50, align 8
  %52 = getelementptr %TaskArg.10, ptr %3, i64 16
  %53 = getelementptr %TaskArg.10, ptr %52, i32 0, i32 0
  %54 = load ptr, ptr %53, align 8
  %55 = getelementptr %TaskArg.10, ptr %3, i64 17
  %56 = getelementptr %TaskArg.10, ptr %55, i32 0, i32 0
  %57 = load ptr, ptr %56, align 8
  %58 = getelementptr %TaskArg.10, ptr %3, i64 18
  %59 = getelementptr %TaskArg.10, ptr %58, i32 0, i32 0
  %60 = load ptr, ptr %59, align 8
  %61 = getelementptr %TaskArg.10, ptr %3, i64 19
  %62 = getelementptr %TaskArg.10, ptr %61, i32 0, i32 0
  %63 = load ptr, ptr %62, align 8
  %64 = getelementptr %TaskArg.10, ptr %3, i64 20
  %65 = getelementptr %TaskArg.10, ptr %64, i32 0, i32 0
  %66 = load ptr, ptr %65, align 8
  %67 = getelementptr %TaskArg.10, ptr %3, i64 21
  %68 = getelementptr %TaskArg.10, ptr %67, i32 0, i32 0
  %69 = load ptr, ptr %68, align 8
  %70 = getelementptr %TaskArg.10, ptr %3, i64 22
  %71 = getelementptr %TaskArg.10, ptr %70, i32 0, i32 0
  %72 = load ptr, ptr %71, align 8
  %73 = getelementptr %TaskArg.10, ptr %3, i64 23
  %74 = getelementptr %TaskArg.10, ptr %73, i32 0, i32 0
  %75 = load ptr, ptr %74, align 8
  %76 = getelementptr %TaskArg.10, ptr %3, i64 24
  %77 = getelementptr %TaskArg.10, ptr %76, i32 0, i32 0
  %78 = load ptr, ptr %77, align 8
  %79 = getelementptr %TaskArg.10, ptr %3, i64 25
  %80 = getelementptr %TaskArg.10, ptr %79, i32 0, i32 0
  %81 = load ptr, ptr %80, align 8
  %82 = getelementptr %TaskArg.10, ptr %3, i64 26
  %83 = getelementptr %TaskArg.10, ptr %82, i32 0, i32 0
  %84 = load ptr, ptr %83, align 8
  %85 = getelementptr %TaskArg.10, ptr %3, i64 27
  %86 = getelementptr %TaskArg.10, ptr %85, i32 0, i32 0
  %87 = load ptr, ptr %86, align 8
  %88 = getelementptr %TaskArg.10, ptr %3, i64 28
  %89 = getelementptr %TaskArg.10, ptr %88, i32 0, i32 0
  %90 = load ptr, ptr %89, align 8
  %91 = getelementptr %TaskArg.10, ptr %3, i64 29
  %92 = getelementptr %TaskArg.10, ptr %91, i32 0, i32 0
  %93 = load ptr, ptr %92, align 8
  %94 = getelementptr %TaskArg.10, ptr %3, i64 30
  %95 = getelementptr %TaskArg.10, ptr %94, i32 0, i32 0
  %96 = load ptr, ptr %95, align 8
  %97 = getelementptr %TaskArg.10, ptr %3, i64 31
  %98 = getelementptr %TaskArg.10, ptr %97, i32 0, i32 0
  %99 = load ptr, ptr %98, align 8
  %100 = getelementptr %TaskArg.10, ptr %3, i64 32
  %101 = getelementptr %TaskArg.10, ptr %100, i32 0, i32 0
  %102 = load ptr, ptr %101, align 8
  %103 = getelementptr %TaskArg.10, ptr %3, i64 33
  %104 = getelementptr %TaskArg.10, ptr %103, i32 0, i32 0
  %105 = load ptr, ptr %104, align 8
  %106 = getelementptr %TaskArg.10, ptr %3, i64 34
  %107 = getelementptr %TaskArg.10, ptr %106, i32 0, i32 0
  %108 = load ptr, ptr %107, align 8
  %109 = getelementptr %TaskArg.10, ptr %3, i64 35
  %110 = getelementptr %TaskArg.10, ptr %109, i32 0, i32 0
  %111 = load ptr, ptr %110, align 8
  %112 = getelementptr %TaskArg.10, ptr %3, i64 36
  %113 = getelementptr %TaskArg.10, ptr %112, i32 0, i32 0
  %114 = load ptr, ptr %113, align 8
  %115 = getelementptr %TaskArg.10, ptr %3, i64 37
  %116 = getelementptr %TaskArg.10, ptr %115, i32 0, i32 0
  %117 = load ptr, ptr %116, align 8
  %118 = getelementptr %TaskArg.10, ptr %3, i64 38
  %119 = getelementptr %TaskArg.10, ptr %118, i32 0, i32 0
  %120 = load ptr, ptr %119, align 8
  %121 = getelementptr %TaskArg.10, ptr %3, i64 39
  %122 = getelementptr %TaskArg.10, ptr %121, i32 0, i32 0
  %123 = load ptr, ptr %122, align 8
  %124 = getelementptr %TaskArg.10, ptr %3, i64 40
  %125 = getelementptr %TaskArg.10, ptr %124, i32 0, i32 0
  %126 = load ptr, ptr %125, align 8
  %127 = getelementptr %TaskArg.10, ptr %3, i64 41
  %128 = getelementptr %TaskArg.10, ptr %127, i32 0, i32 0
  %129 = load ptr, ptr %128, align 8
  %130 = getelementptr %TaskArg.10, ptr %3, i64 42
  %131 = getelementptr %TaskArg.10, ptr %130, i32 0, i32 0
  %132 = load ptr, ptr %131, align 8
  %133 = getelementptr %TaskArg.10, ptr %3, i64 43
  %134 = getelementptr %TaskArg.10, ptr %133, i32 0, i32 0
  %135 = load ptr, ptr %134, align 8
  %136 = getelementptr %TaskArg.10, ptr %3, i64 44
  %137 = getelementptr %TaskArg.10, ptr %136, i32 0, i32 0
  %138 = load ptr, ptr %137, align 8
  %139 = getelementptr %TaskArg.10, ptr %3, i64 45
  %140 = getelementptr %TaskArg.10, ptr %139, i32 0, i32 0
  %141 = load ptr, ptr %140, align 8
  %142 = getelementptr %TaskArg.10, ptr %3, i64 46
  %143 = getelementptr %TaskArg.10, ptr %142, i32 0, i32 0
  %144 = load ptr, ptr %143, align 8
  %145 = getelementptr %TaskArg.10, ptr %3, i64 47
  %146 = getelementptr %TaskArg.10, ptr %145, i32 0, i32 0
  %147 = load ptr, ptr %146, align 8
  %148 = getelementptr %TaskArg.10, ptr %3, i64 48
  %149 = getelementptr %TaskArg.10, ptr %148, i32 0, i32 0
  %150 = load ptr, ptr %149, align 8
  %151 = getelementptr %TaskArg.10, ptr %3, i64 49
  %152 = getelementptr %TaskArg.10, ptr %151, i32 0, i32 0
  %153 = load ptr, ptr %152, align 8
  %154 = getelementptr %TaskArg.10, ptr %3, i64 50
  %155 = getelementptr %TaskArg.10, ptr %154, i32 0, i32 0
  %156 = load ptr, ptr %155, align 8
  %157 = getelementptr %TaskArg.10, ptr %3, i64 51
  %158 = getelementptr %TaskArg.10, ptr %157, i32 0, i32 0
  %159 = load ptr, ptr %158, align 8
  %160 = getelementptr %TaskArg.10, ptr %3, i64 52
  %161 = getelementptr %TaskArg.10, ptr %160, i32 0, i32 0
  %162 = load ptr, ptr %161, align 8
  call void @taskflow_task_10(ptr %6, ptr %9, ptr %12, ptr %15, ptr %18, ptr %21, ptr %24, ptr %27, ptr %30, ptr %33, ptr %36, ptr %39, ptr %42, ptr %45, ptr %48, ptr %51, ptr %54, ptr %57, ptr %60, ptr %63, ptr %66, ptr %69, ptr %72, ptr %75, ptr %78, ptr %81, ptr %84, ptr %87, ptr %90, ptr %93, ptr %96, ptr %99, ptr %102, ptr %105, ptr %108, ptr %111, ptr %114, ptr %117, ptr %120, ptr %123, ptr %126, ptr %129, ptr %132, ptr %135, ptr %138, ptr %141, ptr %144, ptr %147, ptr %150, ptr %153, ptr %156, ptr %159, ptr %162)
  ret void
}

define internal void @taskflow_task_wrapper_25(ptr %0) {
  %2 = getelementptr %TaskArgs.25, ptr %0, i32 0, i32 0
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr %TaskArg.25, ptr %3, i64 0
  %5 = getelementptr %TaskArg.25, ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr %TaskArg.25, ptr %3, i64 1
  %8 = getelementptr %TaskArg.25, ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  call void @taskflow_task_25(ptr %6, ptr %9)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #5

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "use-soft-float"="false" }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "use-soft-float"="false" }
attributes #4 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-jump-tables"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "use-soft-float"="false" }
attributes #5 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
