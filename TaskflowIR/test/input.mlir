llvm.func @main() -> (){
    llvm.call @tf_some_function() : ()->()
    llvm.call @tf_another_function() : ()->()
    llvm.return
}


llvm.func @tf_some_function() -> ()
llvm.func @tf_another_function() -> ()