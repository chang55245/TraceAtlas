llvm.func @main() -> (){
    llvm.call @tf_some_function() : ()->()
    llvm.call @unextractable_function_0() : ()->()
    llvm.call @unextractable_function_1() : ()->()
    llvm.call @unextractable_function_2() : ()->()
    llvm.call @tf_another_function() : ()->()
    llvm.return
}

llvm.func @tf_some_function() -> ()
llvm.func @tf_another_function() -> ()
llvm.func @unextractable_function_0() -> ()
llvm.func @unextractable_function_1() -> ()
llvm.func @unextractable_function_2() -> ()
