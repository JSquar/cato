// RUN: ${CATO_ROOT}/src/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <stdlib.h>

int main()
{
    int* ptr = malloc(sizeof(int));

    *ptr = 42;

    printf("Value: %d\n", *ptr);

    free(ptr);
}

/*
; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %0 = alloca i32
  call void @_Z15cato_initializev()
  br label %entry.split

entry.split:                                      ; preds = %entry
  %call = call i8* @_Z22allocate_shared_memoryli(i64 4, i32 1275068685)
  %1 = bitcast i8* %call to i32*
  store i32 42, i32* %1, align 4, !tbaa !2
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i32 0, i32 0), i32 42)
  call void @free(i8* %call) #3
  store i32 0, i32* %0
  br label %cato_finalize

cato_finalize:                                    ; preds = %entry.split
  call void @_Z13cato_finalizev()
  %2 = load i32, i32* %0
  ret i32 %2
}
*/
