; ModuleID = '/heorot/lchang21/taskflow/taskflow/taskflow-lib/test.cpp'
source_filename = "/heorot/lchang21/taskflow/taskflow/taskflow-lib/test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.TaskArgs = type { ptr, i64 }
%struct.TaskArg = type { ptr, i8 }

@.str = private unnamed_addr constant [15 x i8] c"Int value: %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [19 x i8] c"Float value: %.2f\0A\00", align 1
@.str.2 = private unnamed_addr constant [12 x i8] c"String: %s\0A\00", align 1
@.str.3 = private unnamed_addr constant [26 x i8] c"Array values: %d, %d, %d\0A\00", align 1
@.str.4 = private unnamed_addr constant [25 x i8] c"Math task: %d + %d = %d\0A\00", align 1
@.str.5 = private unnamed_addr constant [38 x i8] c"String task: combining '%s' and '%s'\0A\00", align 1
@.str.6 = private unnamed_addr constant [15 x i8] c"Array sum: %d\0A\00", align 1
@.str.7 = private unnamed_addr constant [25 x i8] c"Final task: result = %d\0A\00", align 1
@.str.8 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@__const.main.array = private unnamed_addr constant [3 x i32] [i32 1, i32 2, i32 3], align 4
@.str.9 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@__const.main.numbers = private unnamed_addr constant [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5], align 16
@.str.10 = private unnamed_addr constant [10 x i8] c"math_task\00", align 1
@.str.11 = private unnamed_addr constant [12 x i8] c"string_task\00", align 1
@.str.12 = private unnamed_addr constant [11 x i8] c"array_task\00", align 1
@.str.13 = private unnamed_addr constant [11 x i8] c"final_task\00", align 1
@.str.14 = private unnamed_addr constant [24 x i8] c"\0AExecuting taskflow...\0A\00", align 1
@.str.15 = private unnamed_addr constant [21 x i8] c"-------------------\0A\00", align 1
@.str.16 = private unnamed_addr constant [10 x i8] c"Results:\0A\00", align 1
@.str.17 = private unnamed_addr constant [17 x i8] c"Math result: %d\0A\00", align 1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z23task_with_multiple_argsP8TaskArgs(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %7 = load ptr, ptr %2, align 8
  %8 = getelementptr inbounds %struct.TaskArgs, ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds %struct.TaskArg, ptr %9, i64 0
  %11 = getelementptr inbounds %struct.TaskArg, ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  store ptr %12, ptr %3, align 8
  %13 = load ptr, ptr %2, align 8
  %14 = getelementptr inbounds %struct.TaskArgs, ptr %13, i32 0, i32 0
  %15 = load ptr, ptr %14, align 8
  %16 = getelementptr inbounds %struct.TaskArg, ptr %15, i64 1
  %17 = getelementptr inbounds %struct.TaskArg, ptr %16, i32 0, i32 0
  %18 = load ptr, ptr %17, align 8
  store ptr %18, ptr %4, align 8
  %19 = load ptr, ptr %2, align 8
  %20 = getelementptr inbounds %struct.TaskArgs, ptr %19, i32 0, i32 0
  %21 = load ptr, ptr %20, align 8
  %22 = getelementptr inbounds %struct.TaskArg, ptr %21, i64 2
  %23 = getelementptr inbounds %struct.TaskArg, ptr %22, i32 0, i32 0
  %24 = load ptr, ptr %23, align 8
  store ptr %24, ptr %5, align 8
  %25 = load ptr, ptr %2, align 8
  %26 = getelementptr inbounds %struct.TaskArgs, ptr %25, i32 0, i32 0
  %27 = load ptr, ptr %26, align 8
  %28 = getelementptr inbounds %struct.TaskArg, ptr %27, i64 3
  %29 = getelementptr inbounds %struct.TaskArg, ptr %28, i32 0, i32 0
  %30 = load ptr, ptr %29, align 8
  store ptr %30, ptr %6, align 8
  %31 = load ptr, ptr %3, align 8
  %32 = load i32, ptr %31, align 4
  %33 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %32)
  %34 = load ptr, ptr %4, align 8
  %35 = load float, ptr %34, align 4
  %36 = fpext float %35 to double
  %37 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, double noundef %36)
  %38 = load ptr, ptr %5, align 8
  %39 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, ptr noundef %38)
  %40 = load ptr, ptr %6, align 8
  %41 = getelementptr inbounds i32, ptr %40, i64 0
  %42 = load i32, ptr %41, align 4
  %43 = load ptr, ptr %6, align 8
  %44 = getelementptr inbounds i32, ptr %43, i64 1
  %45 = load i32, ptr %44, align 4
  %46 = load ptr, ptr %6, align 8
  %47 = getelementptr inbounds i32, ptr %46, i64 2
  %48 = load i32, ptr %47, align 4
  %49 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i32 noundef %42, i32 noundef %45, i32 noundef %48)
  ret void
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z16direct_math_taskiiPi(i32 noundef %0, i32 noundef %1, ptr noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  store i32 %0, ptr %4, align 4
  store i32 %1, ptr %5, align 4
  store ptr %2, ptr %6, align 8
  %7 = load i32, ptr %4, align 4
  %8 = load i32, ptr %5, align 4
  %9 = add nsw i32 %7, %8
  %10 = load ptr, ptr %6, align 8
  store i32 %9, ptr %10, align 4
  %11 = load i32, ptr %4, align 4
  %12 = load i32, ptr %5, align 4
  %13 = load ptr, ptr %6, align 8
  %14 = load i32, ptr %13, align 4
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str.4, i32 noundef %11, i32 noundef %12, i32 noundef %14)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z17math_task_wrapperP8TaskArgs(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %6 = load ptr, ptr %2, align 8
  %7 = getelementptr inbounds %struct.TaskArgs, ptr %6, i32 0, i32 0
  %8 = load ptr, ptr %7, align 8
  %9 = getelementptr inbounds %struct.TaskArg, ptr %8, i64 0
  %10 = getelementptr inbounds %struct.TaskArg, ptr %9, i32 0, i32 0
  %11 = load ptr, ptr %10, align 8
  %12 = load i32, ptr %11, align 4
  store i32 %12, ptr %3, align 4
  %13 = load ptr, ptr %2, align 8
  %14 = getelementptr inbounds %struct.TaskArgs, ptr %13, i32 0, i32 0
  %15 = load ptr, ptr %14, align 8
  %16 = getelementptr inbounds %struct.TaskArg, ptr %15, i64 1
  %17 = getelementptr inbounds %struct.TaskArg, ptr %16, i32 0, i32 0
  %18 = load ptr, ptr %17, align 8
  %19 = load i32, ptr %18, align 4
  store i32 %19, ptr %4, align 4
  %20 = load ptr, ptr %2, align 8
  %21 = getelementptr inbounds %struct.TaskArgs, ptr %20, i32 0, i32 0
  %22 = load ptr, ptr %21, align 8
  %23 = getelementptr inbounds %struct.TaskArg, ptr %22, i64 2
  %24 = getelementptr inbounds %struct.TaskArg, ptr %23, i32 0, i32 0
  %25 = load ptr, ptr %24, align 8
  store ptr %25, ptr %5, align 8
  %26 = load i32, ptr %3, align 4
  %27 = load i32, ptr %4, align 4
  %28 = load ptr, ptr %5, align 8
  call void @_Z16direct_math_taskiiPi(i32 noundef %26, i32 noundef %27, ptr noundef %28)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z18direct_string_taskPKcS0_(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load ptr, ptr %4, align 8
  %7 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, ptr noundef %5, ptr noundef %6)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z19string_task_wrapperP8TaskArgs(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %5 = load ptr, ptr %2, align 8
  %6 = getelementptr inbounds %struct.TaskArgs, ptr %5, i32 0, i32 0
  %7 = load ptr, ptr %6, align 8
  %8 = getelementptr inbounds %struct.TaskArg, ptr %7, i64 0
  %9 = getelementptr inbounds %struct.TaskArg, ptr %8, i32 0, i32 0
  %10 = load ptr, ptr %9, align 8
  store ptr %10, ptr %3, align 8
  %11 = load ptr, ptr %2, align 8
  %12 = getelementptr inbounds %struct.TaskArgs, ptr %11, i32 0, i32 0
  %13 = load ptr, ptr %12, align 8
  %14 = getelementptr inbounds %struct.TaskArg, ptr %13, i64 1
  %15 = getelementptr inbounds %struct.TaskArg, ptr %14, i32 0, i32 0
  %16 = load ptr, ptr %15, align 8
  store ptr %16, ptr %4, align 8
  %17 = load ptr, ptr %3, align 8
  %18 = load ptr, ptr %4, align 8
  call void @_Z18direct_string_taskPKcS0_(ptr noundef %17, ptr noundef %18)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z17direct_array_taskPimS_(ptr noundef %0, i64 noundef %1, ptr noundef %2) #0 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  %8 = load ptr, ptr %6, align 8
  store i32 0, ptr %8, align 4
  store i64 0, ptr %7, align 8
  br label %9

9:                                                ; preds = %21, %3
  %10 = load i64, ptr %7, align 8
  %11 = load i64, ptr %5, align 8
  %12 = icmp ult i64 %10, %11
  br i1 %12, label %13, label %24

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 8
  %15 = load i64, ptr %7, align 8
  %16 = getelementptr inbounds i32, ptr %14, i64 %15
  %17 = load i32, ptr %16, align 4
  %18 = load ptr, ptr %6, align 8
  %19 = load i32, ptr %18, align 4
  %20 = add nsw i32 %19, %17
  store i32 %20, ptr %18, align 4
  br label %21

21:                                               ; preds = %13
  %22 = load i64, ptr %7, align 8
  %23 = add i64 %22, 1
  store i64 %23, ptr %7, align 8
  br label %9, !llvm.loop !6

24:                                               ; preds = %9
  %25 = load ptr, ptr %6, align 8
  %26 = load i32, ptr %25, align 4
  %27 = call i32 (ptr, ...) @printf(ptr noundef @.str.6, i32 noundef %26)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z18array_task_wrapperP8TaskArgs(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %6 = load ptr, ptr %2, align 8
  %7 = getelementptr inbounds %struct.TaskArgs, ptr %6, i32 0, i32 0
  %8 = load ptr, ptr %7, align 8
  %9 = getelementptr inbounds %struct.TaskArg, ptr %8, i64 0
  %10 = getelementptr inbounds %struct.TaskArg, ptr %9, i32 0, i32 0
  %11 = load ptr, ptr %10, align 8
  store ptr %11, ptr %3, align 8
  %12 = load ptr, ptr %2, align 8
  %13 = getelementptr inbounds %struct.TaskArgs, ptr %12, i32 0, i32 0
  %14 = load ptr, ptr %13, align 8
  %15 = getelementptr inbounds %struct.TaskArg, ptr %14, i64 1
  %16 = getelementptr inbounds %struct.TaskArg, ptr %15, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %17, align 8
  store i64 %18, ptr %4, align 8
  %19 = load ptr, ptr %2, align 8
  %20 = getelementptr inbounds %struct.TaskArgs, ptr %19, i32 0, i32 0
  %21 = load ptr, ptr %20, align 8
  %22 = getelementptr inbounds %struct.TaskArg, ptr %21, i64 2
  %23 = getelementptr inbounds %struct.TaskArg, ptr %22, i32 0, i32 0
  %24 = load ptr, ptr %23, align 8
  store ptr %24, ptr %5, align 8
  %25 = load ptr, ptr %3, align 8
  %26 = load i64, ptr %4, align 8
  %27 = load ptr, ptr %5, align 8
  call void @_Z17direct_array_taskPimS_(ptr noundef %25, i64 noundef %26, ptr noundef %27)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z10final_taskP8TaskArgs(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %4 = load ptr, ptr %2, align 8
  %5 = getelementptr inbounds %struct.TaskArgs, ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %struct.TaskArg, ptr %6, i64 0
  %8 = getelementptr inbounds %struct.TaskArg, ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  store ptr %9, ptr %3, align 8
  %10 = load ptr, ptr %3, align 8
  %11 = load i32, ptr %10, align 4
  %12 = call i32 (ptr, ...) @printf(ptr noundef @.str.7, i32 noundef %11)
  ret void
}

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #2 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  %4 = alloca float, align 4
  %5 = alloca ptr, align 8
  %6 = alloca [3 x i32], align 4
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  %15 = alloca [5 x i32], align 16
  %16 = alloca i64, align 8
  %17 = alloca i32, align 4
  %18 = alloca ptr, align 8
  %19 = alloca ptr, align 8
  %20 = alloca ptr, align 8
  %21 = alloca ptr, align 8
  %22 = alloca ptr, align 8
  %23 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
  %24 = call ptr @taskflow_create()
  store ptr %24, ptr %2, align 8
  store i32 42, ptr %3, align 4
  store float 0x400921FA00000000, ptr %4, align 4
  store ptr @.str.8, ptr %5, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %6, ptr align 4 @__const.main.array, i64 12, i1 false)
  %25 = call ptr @create_task_args(i64 noundef 4)
  store ptr %25, ptr %7, align 8
  %26 = load ptr, ptr %7, align 8
  call void @set_task_arg_ptr(ptr noundef %26, i64 noundef 0, ptr noundef %3)
  %27 = load ptr, ptr %7, align 8
  call void @set_task_arg_ptr(ptr noundef %27, i64 noundef 1, ptr noundef %4)
  %28 = load ptr, ptr %7, align 8
  %29 = load ptr, ptr %5, align 8
  call void @set_task_arg_ptr(ptr noundef %28, i64 noundef 2, ptr noundef %29)
  %30 = load ptr, ptr %7, align 8
  %31 = getelementptr inbounds [3 x i32], ptr %6, i64 0, i64 0
  call void @set_task_arg_ptr(ptr noundef %30, i64 noundef 3, ptr noundef %31)
  store i32 10, ptr %8, align 4
  store i32 20, ptr %9, align 4
  store i32 0, ptr %10, align 4
  %32 = call ptr @create_task_args(i64 noundef 3)
  store ptr %32, ptr %11, align 8
  %33 = load ptr, ptr %11, align 8
  call void @set_task_arg_ptr(ptr noundef %33, i64 noundef 0, ptr noundef %8)
  %34 = load ptr, ptr %11, align 8
  call void @set_task_arg_ptr(ptr noundef %34, i64 noundef 1, ptr noundef %9)
  %35 = load ptr, ptr %11, align 8
  call void @set_task_arg_ptr(ptr noundef %35, i64 noundef 2, ptr noundef %10)
  store ptr @.str.8, ptr %12, align 8
  store ptr @.str.9, ptr %13, align 8
  %36 = call ptr @create_task_args(i64 noundef 2)
  store ptr %36, ptr %14, align 8
  %37 = load ptr, ptr %14, align 8
  %38 = load ptr, ptr %12, align 8
  call void @set_task_arg_ptr(ptr noundef %37, i64 noundef 0, ptr noundef %38)
  %39 = load ptr, ptr %14, align 8
  %40 = load ptr, ptr %13, align 8
  call void @set_task_arg_ptr(ptr noundef %39, i64 noundef 1, ptr noundef %40)
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %15, ptr align 16 @__const.main.numbers, i64 20, i1 false)
  store i64 5, ptr %16, align 8
  store i32 0, ptr %17, align 4
  %41 = call ptr @create_task_args(i64 noundef 3)
  store ptr %41, ptr %18, align 8
  %42 = load ptr, ptr %18, align 8
  %43 = getelementptr inbounds [5 x i32], ptr %15, i64 0, i64 0
  call void @set_task_arg_ptr(ptr noundef %42, i64 noundef 0, ptr noundef %43)
  %44 = load ptr, ptr %18, align 8
  call void @set_task_arg_ptr(ptr noundef %44, i64 noundef 1, ptr noundef %16)
  %45 = load ptr, ptr %18, align 8
  call void @set_task_arg_ptr(ptr noundef %45, i64 noundef 2, ptr noundef %17)
  %46 = call ptr @create_task_args(i64 noundef 1)
  store ptr %46, ptr %19, align 8
  %47 = load ptr, ptr %19, align 8
  call void @set_task_arg_ptr(ptr noundef %47, i64 noundef 0, ptr noundef %17)
  %48 = load ptr, ptr %2, align 8
  %49 = load ptr, ptr %11, align 8
  %50 = call ptr @taskflow_create_task(ptr noundef %48, ptr noundef @.str.10, ptr noundef @_Z17math_task_wrapperP8TaskArgs, ptr noundef %49)
  store ptr %50, ptr %20, align 8
  %51 = load ptr, ptr %2, align 8
  %52 = load ptr, ptr %14, align 8
  %53 = call ptr @taskflow_create_task(ptr noundef %51, ptr noundef @.str.11, ptr noundef @_Z19string_task_wrapperP8TaskArgs, ptr noundef %52)
  store ptr %53, ptr %21, align 8
  %54 = load ptr, ptr %2, align 8
  %55 = load ptr, ptr %18, align 8
  %56 = call ptr @taskflow_create_task(ptr noundef %54, ptr noundef @.str.12, ptr noundef @_Z18array_task_wrapperP8TaskArgs, ptr noundef %55)
  store ptr %56, ptr %22, align 8
  %57 = load ptr, ptr %2, align 8
  %58 = load ptr, ptr %19, align 8
  %59 = call ptr @taskflow_create_task(ptr noundef %57, ptr noundef @.str.13, ptr noundef @_Z10final_taskP8TaskArgs, ptr noundef %58)
  store ptr %59, ptr %23, align 8
  %60 = load ptr, ptr %20, align 8
  %61 = load ptr, ptr %21, align 8
  call void @taskflow_add_dependency(ptr noundef %60, ptr noundef %61)
  %62 = load ptr, ptr %21, align 8
  %63 = load ptr, ptr %22, align 8
  call void @taskflow_add_dependency(ptr noundef %62, ptr noundef %63)
  %64 = load ptr, ptr %22, align 8
  %65 = load ptr, ptr %23, align 8
  call void @taskflow_add_dependency(ptr noundef %64, ptr noundef %65)
  %66 = call i32 (ptr, ...) @printf(ptr noundef @.str.14)
  %67 = call i32 (ptr, ...) @printf(ptr noundef @.str.15)
  %68 = load ptr, ptr %2, align 8
  call void @taskflow_execute(ptr noundef %68)
  %69 = call i32 (ptr, ...) @printf(ptr noundef @.str.15)
  %70 = call i32 (ptr, ...) @printf(ptr noundef @.str.16)
  %71 = load i32, ptr %10, align 4
  %72 = call i32 (ptr, ...) @printf(ptr noundef @.str.17, i32 noundef %71)
  %73 = load i32, ptr %17, align 4
  %74 = call i32 (ptr, ...) @printf(ptr noundef @.str.6, i32 noundef %73)
  %75 = call i32 (ptr, ...) @printf(ptr noundef @.str.15)
  %76 = load ptr, ptr %7, align 8
  call void @destroy_task_args(ptr noundef %76)
  %77 = load ptr, ptr %11, align 8
  call void @destroy_task_args(ptr noundef %77)
  %78 = load ptr, ptr %14, align 8
  call void @destroy_task_args(ptr noundef %78)
  %79 = load ptr, ptr %18, align 8
  call void @destroy_task_args(ptr noundef %79)
  %80 = load ptr, ptr %19, align 8
  call void @destroy_task_args(ptr noundef %80)
  %81 = load ptr, ptr %2, align 8
  call void @taskflow_destroy(ptr noundef %81)
  ret i32 0
}

declare ptr @taskflow_create() #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

declare ptr @create_task_args(i64 noundef) #1

declare void @set_task_arg_ptr(ptr noundef, i64 noundef, ptr noundef) #1

declare ptr @taskflow_create_task(ptr noundef, ptr noundef, ptr noundef, ptr noundef) #1

declare void @taskflow_add_dependency(ptr noundef, ptr noundef) #1

declare void @taskflow_execute(ptr noundef) #1

declare void @destroy_task_args(ptr noundef) #1

declare void @taskflow_destroy(ptr noundef) #1

attributes #0 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6 (https://github.com/llvm/llvm-project 6009708b4367171ccdbf4b5905cb6a803753fe18)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
