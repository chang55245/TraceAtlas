; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

%struct.timespec = type { i64, i64 }

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
  %start = alloca ptr, align 8
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
  %a = call ptr @taskflow_create()
  store ptr %a, ptr %start, align 8
  %b = load ptr, ptr %start, align 8
  %224 = call ptr @taskflow_create_task(ptr %b, ptr @func_name_1, ptr @taskflow_task_1)
  br label %225

225:                                              ; preds = %223
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %10)
  %c = load ptr, ptr %start, align 8
  ; %226 = call ptr @taskflow_create_task(ptr %c, ptr @func_name_4, ptr @taskflow_task_4)
  br label %227

227:                                              ; preds = %225
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %10)
  %d = load ptr, ptr %start, align 8
  ; %228 = call ptr @taskflow_create_task(ptr %d, ptr @func_name_7, ptr @taskflow_task_7)
  br label %229

229:                                              ; preds = %227
  call void @llvm.lifetime.start.p0(i64 -1, ptr %4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %5)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %6)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %7)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %8)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %9)
  call void @llvm.lifetime.start.p0(i64 -1, ptr %10)
  %e = load ptr, ptr %start, align 8
  ; %230 = call ptr @taskflow_create_task(ptr %e, ptr @func_name_10, ptr @taskflow_task_10)
  br label %231

231:                                              ; preds = %229
  %f = load ptr, ptr %start, align 8
  ; %232 = call ptr @taskflow_create_task(ptr %f, ptr @func_name_25, ptr @taskflow_task_25)
  ; call void @taskflow_add_dependency(ptr %224, ptr %232)
  ; call void @taskflow_add_dependency(ptr %226, ptr %232)
  ; call void @taskflow_add_dependency(ptr %228, ptr %232)
  ; call void @taskflow_add_dependency(ptr %230, ptr %232)
  %g = load ptr, ptr %start, align 8
  call void @taskflow_execute(ptr %g)
  br label %233

233:                                              ; preds = %231
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

declare ptr @taskflow_create_task(ptr, ptr, ptr)

declare void @taskflow_add_dependency(ptr, ptr)

declare void @taskflow_execute(ptr)

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
