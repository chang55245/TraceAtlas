; ModuleID = '/heorot/lchang21/taskflow/taskflow/taskflow-lib/test.cpp'
source_filename = "/heorot/lchang21/taskflow/taskflow/taskflow-lib/test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [18 x i8] c"Executing Task A\0A\00", align 1
@.str.1 = private unnamed_addr constant [18 x i8] c"Executing Task B\0A\00", align 1
@.str.2 = private unnamed_addr constant [18 x i8] c"Executing Task C\0A\00", align 1
@.str.3 = private unnamed_addr constant [18 x i8] c"Executing Task D\0A\00", align 1
@.str.4 = private unnamed_addr constant [2 x i8] c"A\00", align 1
@.str.5 = private unnamed_addr constant [2 x i8] c"B\00", align 1
@.str.6 = private unnamed_addr constant [2 x i8] c"C\00", align 1
@.str.7 = private unnamed_addr constant [2 x i8] c"D\00", align 1
@.str.8 = private unnamed_addr constant [21 x i8] c"Number of tasks: %d\0A\00", align 1
@.str.9 = private unnamed_addr constant [23 x i8] c"Is taskflow empty? %s\0A\00", align 1
@.str.10 = private unnamed_addr constant [4 x i8] c"yes\00", align 1
@.str.11 = private unnamed_addr constant [3 x i8] c"no\00", align 1
@.str.12 = private unnamed_addr constant [24 x i8] c"\0AExecuting taskflow...\0A\00", align 1
@.str.13 = private unnamed_addr constant [21 x i8] c"-------------------\0A\00", align 1
@.str.14 = private unnamed_addr constant [21 x i8] c"Execution complete!\0A\00", align 1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z5taskAv() #0 {
  %1 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  ret void
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z5taskBv() #0 {
  %1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z5taskCv() #0 {
  %1 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z5taskDv() #0 {
  %1 = call i32 (ptr, ...) @printf(ptr noundef @.str.3)
  ret void
}

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #2 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
  %7 = call ptr @taskflow_create()
  store ptr %7, ptr %2, align 8
  %8 = load ptr, ptr %2, align 8
  %9 = call ptr @taskflow_create_task(ptr noundef %8, ptr noundef @.str.4, ptr noundef @_Z5taskAv)
  store ptr %9, ptr %3, align 8
  %10 = load ptr, ptr %2, align 8
  %11 = call ptr @taskflow_create_task(ptr noundef %10, ptr noundef @.str.5, ptr noundef @_Z5taskBv)
  store ptr %11, ptr %4, align 8
  %12 = load ptr, ptr %2, align 8
  %13 = call ptr @taskflow_create_task(ptr noundef %12, ptr noundef @.str.6, ptr noundef @_Z5taskCv)
  store ptr %13, ptr %5, align 8
  %14 = load ptr, ptr %2, align 8
  %15 = call ptr @taskflow_create_task(ptr noundef %14, ptr noundef @.str.7, ptr noundef @_Z5taskDv)
  store ptr %15, ptr %6, align 8
  %16 = load ptr, ptr %3, align 8
  %17 = load ptr, ptr %4, align 8
  call void @taskflow_add_dependency(ptr noundef %16, ptr noundef %17)
  %18 = load ptr, ptr %3, align 8
  %19 = load ptr, ptr %5, align 8
  call void @taskflow_add_dependency(ptr noundef %18, ptr noundef %19)
  %20 = load ptr, ptr %4, align 8
  %21 = load ptr, ptr %6, align 8
  call void @taskflow_add_dependency(ptr noundef %20, ptr noundef %21)
  %22 = load ptr, ptr %5, align 8
  %23 = load ptr, ptr %6, align 8
  call void @taskflow_add_dependency(ptr noundef %22, ptr noundef %23)
  %24 = load ptr, ptr %2, align 8
  %25 = call i32 @taskflow_num_tasks(ptr noundef %24)
  %26 = call i32 (ptr, ...) @printf(ptr noundef @.str.8, i32 noundef %25)
  %27 = load ptr, ptr %2, align 8
  %28 = call i32 @taskflow_is_empty(ptr noundef %27)
  %29 = icmp ne i32 %28, 0
  %30 = zext i1 %29 to i64
  %31 = select i1 %29, ptr @.str.10, ptr @.str.11
  %32 = call i32 (ptr, ...) @printf(ptr noundef @.str.9, ptr noundef %31)
  %33 = call i32 (ptr, ...) @printf(ptr noundef @.str.12)
  %34 = call i32 (ptr, ...) @printf(ptr noundef @.str.13)
  %35 = load ptr, ptr %2, align 8
  call void @taskflow_execute(ptr noundef %35)
  %36 = call i32 (ptr, ...) @printf(ptr noundef @.str.13)
  %37 = call i32 (ptr, ...) @printf(ptr noundef @.str.14)
  %38 = load ptr, ptr %2, align 8
  call void @taskflow_destroy(ptr noundef %38)
  ret i32 0
}

declare ptr @taskflow_create() #1

declare ptr @taskflow_create_task(ptr noundef, ptr noundef, ptr noundef) #1

declare void @taskflow_add_dependency(ptr noundef, ptr noundef) #1

declare i32 @taskflow_num_tasks(ptr noundef) #1

declare i32 @taskflow_is_empty(ptr noundef) #1

declare void @taskflow_execute(ptr noundef) #1

declare void @taskflow_destroy(ptr noundef) #1

attributes #0 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6 (https://github.com/llvm/llvm-project 6009708b4367171ccdbf4b5905cb6a803753fe18)"}
