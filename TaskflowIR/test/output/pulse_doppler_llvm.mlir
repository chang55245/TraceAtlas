module attributes {dlti.dl_spec = #dlti.dl_spec<#dlti.dl_entry<i1, dense<8> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr, dense<64> : vector<4xi64>>, #dlti.dl_entry<i16, dense<16> : vector<2xi64>>, #dlti.dl_entry<i8, dense<8> : vector<2xi64>>, #dlti.dl_entry<f16, dense<16> : vector<2xi64>>, #dlti.dl_entry<i32, dense<32> : vector<2xi64>>, #dlti.dl_entry<f128, dense<128> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr<270>, dense<32> : vector<4xi64>>, #dlti.dl_entry<f64, dense<64> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr<272>, dense<64> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<271>, dense<32> : vector<4xi64>>, #dlti.dl_entry<i64, dense<64> : vector<2xi64>>, #dlti.dl_entry<f80, dense<128> : vector<2xi64>>, #dlti.dl_entry<i128, dense<128> : vector<2xi64>>, #dlti.dl_entry<"dlti.endianness", "little">, #dlti.dl_entry<"dlti.stack_alignment", 128 : i64>>, llvm.data_layout = "", llvm.target_triple = ""} {
  llvm.mlir.global internal constant @func_name_25("taskflow_task_25") {addr_space = 0 : i32}
  llvm.mlir.global internal constant @func_name_10("taskflow_task_10") {addr_space = 0 : i32}
  llvm.mlir.global internal constant @func_name_7("taskflow_task_7") {addr_space = 0 : i32}
  llvm.mlir.global internal constant @func_name_4("taskflow_task_4") {addr_space = 0 : i32}
  llvm.mlir.global internal constant @func_name_1("taskflow_task_1") {addr_space = 0 : i32}
  llvm.mlir.global external hidden unnamed_addr constant @".str.2"() {addr_space = 0 : i32, alignment = 1 : i64, dso_local} : !llvm.array<27 x i8>
  llvm.mlir.global external hidden unnamed_addr constant @".str.3"() {addr_space = 0 : i32, alignment = 1 : i64, dso_local} : !llvm.array<2 x i8>
  llvm.mlir.global external hidden unnamed_addr constant @".str.4"() {addr_space = 0 : i32, alignment = 1 : i64, dso_local} : !llvm.array<6 x i8>
  llvm.mlir.global external hidden unnamed_addr constant @".str.5"() {addr_space = 0 : i32, alignment = 1 : i64, dso_local} : !llvm.array<23 x i8>
  llvm.mlir.global external hidden unnamed_addr constant @".str.6"() {addr_space = 0 : i32, alignment = 1 : i64, dso_local} : !llvm.array<4 x i8>
  llvm.func @malloc(i64) -> (!llvm.ptr {llvm.noalias}) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @clock_gettime(i32, !llvm.ptr) -> i32 attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @printf(!llvm.ptr, ...) -> i32 attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @main(%arg0: i32, %arg1: !llvm.ptr) -> i32 attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_inline, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["uwtable", "2"], ["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["min-legal-vector-width", "0"], ["no-jump-tables", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false} {
    %0 = llvm.call @taskflow_create() : () -> !llvm.ptr
    %1 = llvm.mlir.constant(1 : i32) : i32
    %2 = llvm.mlir.constant(0 : i32) : i32
    %3 = llvm.mlir.constant(4 : i32) : i32
    %4 = llvm.mlir.addressof @".str.2" : !llvm.ptr
    %5 = llvm.mlir.addressof @".str.3" : !llvm.ptr
    %6 = llvm.mlir.zero : !llvm.ptr
    %7 = llvm.mlir.constant(2048 : i64) : i64
    %8 = llvm.mlir.constant(256 : i64) : i64
    %9 = llvm.mlir.addressof @".str.6" : !llvm.ptr
    %10 = llvm.mlir.addressof @".str.4" : !llvm.ptr
    %11 = llvm.mlir.addressof @".str.5" : !llvm.ptr
    %12 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %13 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %14 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %15 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %16 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %17 = llvm.alloca %1 x i8 {alignment = 1 : i64} : (i32) -> !llvm.ptr
    %18 = llvm.alloca %1 x !llvm.struct<"struct.timespec", (i64, i64)> {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %19 = llvm.alloca %1 x !llvm.struct<"struct.timespec", (i64, i64)> {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %20 = llvm.alloca %1 x !llvm.struct<"struct.timespec", (i64, i64)> {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %21 = llvm.alloca %1 x !llvm.struct<"struct.timespec", (i64, i64)> {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %22 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %23 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %24 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %25 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %26 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %27 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %28 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %29 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %30 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %31 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %32 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %33 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %34 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %35 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %36 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %37 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %38 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %39 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %40 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %41 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %42 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %43 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %44 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %45 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %46 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %47 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %48 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %49 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %50 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %51 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %52 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %53 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %54 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %55 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %56 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %57 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %58 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %59 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %60 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %61 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %62 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %63 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %64 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %65 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %66 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %67 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %68 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %69 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %70 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %71 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %72 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %73 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %74 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %75 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %76 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %77 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %78 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %79 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %80 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %81 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %82 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %83 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %84 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %85 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %86 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %87 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %88 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %89 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %90 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %91 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %92 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %93 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %94 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %95 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %96 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %97 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %98 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %99 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %100 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %101 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %102 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %103 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %104 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %105 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %106 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %107 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %108 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %109 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %110 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %111 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %112 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %113 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %114 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %115 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %116 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %117 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %118 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %119 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %120 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %121 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %122 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %123 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %124 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %125 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %126 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %127 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %128 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %129 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %130 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %131 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %132 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %133 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %134 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %135 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %136 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %137 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %138 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %139 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %140 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %141 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %142 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %143 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %144 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %145 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %146 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %147 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %148 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %149 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %150 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %151 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %152 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %153 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %154 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %155 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %156 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %157 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %158 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %159 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %160 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %161 = llvm.alloca %1 x !llvm.ptr {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %162 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %163 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %164 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %165 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %166 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %167 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %168 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %169 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %170 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %171 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %172 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %173 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %174 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %175 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %176 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %177 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %178 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %179 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %180 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %181 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %182 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %183 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %184 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %185 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %186 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %187 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %188 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %189 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %190 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %191 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %192 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %193 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %194 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %195 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %196 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %197 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %198 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %199 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %200 = llvm.alloca %1 x i64 {alignment = 8 : i64} : (i32) -> !llvm.ptr
    %201 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %202 = llvm.alloca %1 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %203 = llvm.bitcast %2 : i32 to i32
    %204 = llvm.call @clock_gettime(%3, %19) : (i32, !llvm.ptr) -> i32
    %205 = llvm.call @fopen(%4, %5) : (!llvm.ptr, !llvm.ptr) -> !llvm.ptr
    %206 = llvm.bitcast %161 : !llvm.ptr to !llvm.ptr
    llvm.store %205, %161 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    %207 = llvm.bitcast %161 : !llvm.ptr to !llvm.ptr
    %208 = llvm.load %161 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %209 = llvm.icmp "eq" %208, %6 : !llvm.ptr
    llvm.cond_br %209, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    llvm.call @perror(%10) : (!llvm.ptr) -> ()
    %210 = llvm.call @printf(%11, %4) vararg(!llvm.func<i32 (ptr, ...)>) : (!llvm.ptr, !llvm.ptr) -> i32
    llvm.call @exit(%1) : (i32) -> ()
    llvm.unreachable
  ^bb2:  // pred: ^bb0
    %211 = llvm.call @malloc(%7) : (i64) -> !llvm.ptr
    %212 = llvm.bitcast %211 : !llvm.ptr to !llvm.ptr
    %213 = llvm.bitcast %160 : !llvm.ptr to !llvm.ptr
    llvm.store %212, %160 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    %214 = llvm.bitcast %202 : !llvm.ptr to !llvm.ptr
    llvm.store %2, %202 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb3
  ^bb3:  // 2 preds: ^bb2, ^bb4
    %215 = llvm.bitcast %202 : !llvm.ptr to !llvm.ptr
    %216 = llvm.load %202 {alignment = 4 : i64} : !llvm.ptr -> i32
    %217 = llvm.bitcast %159 : !llvm.ptr to !llvm.ptr
    llvm.store %216, %159 {alignment = 4 : i64} : i32, !llvm.ptr
    %218 = llvm.bitcast %159 : !llvm.ptr to !llvm.ptr
    %219 = llvm.load %159 {alignment = 4 : i64} : !llvm.ptr -> i32
    %220 = llvm.sext %219 : i32 to i64
    %221 = llvm.icmp "ult" %220, %8 : i64
    llvm.cond_br %221, ^bb4, ^bb5
  ^bb4:  // pred: ^bb3
    %222 = llvm.bitcast %159 : !llvm.ptr to !llvm.ptr
    %223 = llvm.load %159 {alignment = 4 : i64} : !llvm.ptr -> i32
    %224 = llvm.sext %223 : i32 to i64
    %225 = llvm.bitcast %160 : !llvm.ptr to !llvm.ptr
    %226 = llvm.load %160 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %227 = llvm.getelementptr inbounds %226[%224] : (!llvm.ptr, i64) -> !llvm.ptr, f64
    %228 = llvm.bitcast %161 : !llvm.ptr to !llvm.ptr
    %229 = llvm.load %161 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %230 = llvm.call @__isoc99_fscanf(%229, %9, %227) vararg(!llvm.func<i32 (ptr, ptr, ...)>) : (!llvm.ptr, !llvm.ptr, !llvm.ptr) -> i32
    %231 = llvm.bitcast %159 : !llvm.ptr to !llvm.ptr
    %232 = llvm.load %159 {alignment = 4 : i64} : !llvm.ptr -> i32
    llvm.call @ComputeDump() : () -> ()
    %233 = llvm.add %232, %1 overflow<nsw> : i32
    %234 = llvm.bitcast %158 : !llvm.ptr to !llvm.ptr
    llvm.store %233, %158 {alignment = 4 : i64} : i32, !llvm.ptr
    %235 = llvm.bitcast %158 : !llvm.ptr to !llvm.ptr
    %236 = llvm.load %158 {alignment = 4 : i64} : !llvm.ptr -> i32
    %237 = llvm.bitcast %202 : !llvm.ptr to !llvm.ptr
    llvm.store %236, %202 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb3
  ^bb5:  // pred: ^bb3
    %238 = llvm.bitcast %161 : !llvm.ptr to !llvm.ptr
    %239 = llvm.load %161 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %240 = llvm.call @fclose(%239) : (!llvm.ptr) -> i32
    %241 = llvm.call @clock_gettime(%3, %21) : (i32, !llvm.ptr) -> i32
    llvm.br ^bb6
  ^bb6:  // pred: ^bb5
    llvm.call @NonKernelSplit() vararg(!llvm.func<void (...)>) : () -> ()
    llvm.br ^bb7
  ^bb7:  // pred: ^bb6
    llvm.intr.lifetime.start -1, %12 : !llvm.ptr
    llvm.intr.lifetime.start -1, %13 : !llvm.ptr
    llvm.intr.lifetime.start -1, %14 : !llvm.ptr
    llvm.intr.lifetime.start -1, %15 : !llvm.ptr
    llvm.intr.lifetime.start -1, %16 : !llvm.ptr
    llvm.intr.lifetime.start -1, %17 : !llvm.ptr
    llvm.intr.lifetime.start -1, %18 : !llvm.ptr
    %242 = llvm.mlir.addressof @taskflow_task_1 : !llvm.ptr
    %243 = llvm.mlir.addressof @func_name_1 : !llvm.ptr
    %244 = llvm.mlir.constant(0 : i64) : i64
    %245 = llvm.getelementptr %243[%244, %244] : (!llvm.ptr, i64, i64) -> !llvm.ptr, !llvm.array<15 x i8>
    %246 = llvm.call @taskflow_create_task(%0, %245, %242) : (!llvm.ptr, !llvm.ptr, !llvm.ptr) -> !llvm.ptr
    llvm.br ^bb8
  ^bb8:  // pred: ^bb7
    llvm.intr.lifetime.start -1, %12 : !llvm.ptr
    llvm.intr.lifetime.start -1, %13 : !llvm.ptr
    llvm.intr.lifetime.start -1, %14 : !llvm.ptr
    llvm.intr.lifetime.start -1, %15 : !llvm.ptr
    llvm.intr.lifetime.start -1, %16 : !llvm.ptr
    llvm.intr.lifetime.start -1, %17 : !llvm.ptr
    llvm.intr.lifetime.start -1, %18 : !llvm.ptr
    %247 = llvm.mlir.addressof @taskflow_task_4 : !llvm.ptr
    %248 = llvm.mlir.addressof @func_name_4 : !llvm.ptr
    %249 = llvm.mlir.constant(0 : i64) : i64
    %250 = llvm.getelementptr %248[%249, %249] : (!llvm.ptr, i64, i64) -> !llvm.ptr, !llvm.array<15 x i8>
    %251 = llvm.call @taskflow_create_task(%0, %250, %247) : (!llvm.ptr, !llvm.ptr, !llvm.ptr) -> !llvm.ptr
    llvm.br ^bb9
  ^bb9:  // pred: ^bb8
    llvm.intr.lifetime.start -1, %12 : !llvm.ptr
    llvm.intr.lifetime.start -1, %13 : !llvm.ptr
    llvm.intr.lifetime.start -1, %14 : !llvm.ptr
    llvm.intr.lifetime.start -1, %15 : !llvm.ptr
    llvm.intr.lifetime.start -1, %16 : !llvm.ptr
    llvm.intr.lifetime.start -1, %17 : !llvm.ptr
    llvm.intr.lifetime.start -1, %18 : !llvm.ptr
    %252 = llvm.mlir.addressof @taskflow_task_7 : !llvm.ptr
    %253 = llvm.mlir.addressof @func_name_7 : !llvm.ptr
    %254 = llvm.mlir.constant(0 : i64) : i64
    %255 = llvm.getelementptr %253[%254, %254] : (!llvm.ptr, i64, i64) -> !llvm.ptr, !llvm.array<15 x i8>
    %256 = llvm.call @taskflow_create_task(%0, %255, %252) : (!llvm.ptr, !llvm.ptr, !llvm.ptr) -> !llvm.ptr
    llvm.br ^bb10
  ^bb10:  // pred: ^bb9
    llvm.intr.lifetime.start -1, %12 : !llvm.ptr
    llvm.intr.lifetime.start -1, %13 : !llvm.ptr
    llvm.intr.lifetime.start -1, %14 : !llvm.ptr
    llvm.intr.lifetime.start -1, %15 : !llvm.ptr
    llvm.intr.lifetime.start -1, %16 : !llvm.ptr
    llvm.intr.lifetime.start -1, %17 : !llvm.ptr
    llvm.intr.lifetime.start -1, %18 : !llvm.ptr
    %257 = llvm.mlir.addressof @taskflow_task_10 : !llvm.ptr
    %258 = llvm.mlir.addressof @func_name_10 : !llvm.ptr
    %259 = llvm.mlir.constant(0 : i64) : i64
    %260 = llvm.getelementptr %258[%259, %259] : (!llvm.ptr, i64, i64) -> !llvm.ptr, !llvm.array<16 x i8>
    %261 = llvm.call @taskflow_create_task(%0, %260, %257) : (!llvm.ptr, !llvm.ptr, !llvm.ptr) -> !llvm.ptr
    llvm.br ^bb11
  ^bb11:  // pred: ^bb10
    %262 = llvm.mlir.addressof @taskflow_task_25 : !llvm.ptr
    %263 = llvm.mlir.addressof @func_name_25 : !llvm.ptr
    %264 = llvm.mlir.constant(0 : i64) : i64
    %265 = llvm.getelementptr %263[%264, %264] : (!llvm.ptr, i64, i64) -> !llvm.ptr, !llvm.array<16 x i8>
    %266 = llvm.call @taskflow_create_task(%0, %265, %262) : (!llvm.ptr, !llvm.ptr, !llvm.ptr) -> !llvm.ptr
    %267 = llvm.bitcast %246 : !llvm.ptr to !llvm.ptr
    llvm.call @taskflow_add_dependency(%267, %266) : (!llvm.ptr, !llvm.ptr) -> ()
    %268 = llvm.bitcast %251 : !llvm.ptr to !llvm.ptr
    llvm.call @taskflow_add_dependency(%268, %266) : (!llvm.ptr, !llvm.ptr) -> ()
    %269 = llvm.bitcast %256 : !llvm.ptr to !llvm.ptr
    llvm.call @taskflow_add_dependency(%269, %266) : (!llvm.ptr, !llvm.ptr) -> ()
    %270 = llvm.bitcast %261 : !llvm.ptr to !llvm.ptr
    llvm.call @taskflow_add_dependency(%270, %266) : (!llvm.ptr, !llvm.ptr) -> ()
    llvm.call @taskflow_execute(%0) : (!llvm.ptr) -> ()
    llvm.br ^bb12
  ^bb12:  // pred: ^bb11
    llvm.return %2 : i32
  }
  llvm.func @fopen(!llvm.ptr, !llvm.ptr) -> (!llvm.ptr {llvm.noalias}) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @perror(!llvm.ptr) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @exit(i32) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = ["noreturn", ["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @__isoc99_fscanf(!llvm.ptr, !llvm.ptr, ...) -> i32 attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @fclose(!llvm.ptr) -> i32 attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @NonKernelSplit(...) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_nans_fp_math = false, no_signed_zeros_fp_math = false, passthrough = [["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @ComputeDump()
  llvm.func hidden @taskflow_task_1(!llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_inline, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["uwtable", "2"], ["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["min-legal-vector-width", "0"], ["no-jump-tables", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func hidden @taskflow_task_4(!llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_inline, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["uwtable", "2"], ["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["min-legal-vector-width", "0"], ["no-jump-tables", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func hidden @taskflow_task_7(!llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_inline, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["uwtable", "2"], ["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["min-legal-vector-width", "0"], ["no-jump-tables", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func hidden @taskflow_task_10(!llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr, !llvm.ptr) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_inline, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["uwtable", "2"], ["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["min-legal-vector-width", "0"], ["no-jump-tables", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func hidden @taskflow_task_25(!llvm.ptr, !llvm.ptr) attributes {frame_pointer = #llvm.framePointerKind<all>, no_infs_fp_math = false, no_inline, no_nans_fp_math = false, no_signed_zeros_fp_math = false, no_unwind, passthrough = [["uwtable", "2"], ["correctly-rounded-divide-sqrt-fp-math", "false"], ["disable-tail-calls", "false"], ["less-precise-fpmad", "false"], ["min-legal-vector-width", "0"], ["no-jump-tables", "false"], ["no-trapping-math", "false"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"], ["use-soft-float", "false"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, unsafe_fp_math = false}
  llvm.func @taskflow_create() -> !llvm.ptr
  llvm.func @taskflow_create_task(!llvm.ptr, !llvm.ptr, !llvm.ptr) -> !llvm.ptr
  llvm.func @taskflow_add_dependency(!llvm.ptr, !llvm.ptr)
  llvm.func @taskflow_execute(!llvm.ptr)
}

