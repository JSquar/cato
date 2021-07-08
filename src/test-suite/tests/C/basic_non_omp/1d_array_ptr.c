// RUN: ${CATO_ROOT}/src/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <stdlib.h>

int main()
{
    int* arr = malloc(sizeof(int) * 4);

    arr[0] = 0;
    arr[1] = 1;
    arr[2] = 2;
    arr[3] = 3;

    printf("[%d, %d, %d, %d]\n", arr[0], arr[1], arr[2], arr[3]);

    free(arr);
}

/*
; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %0 = alloca i32
  call void @_Z15cato_initializev()
  br label %entry.split

entry.split:                                      ; preds = %entry
  %call = call i8* @_Z22allocate_shared_memoryli(i64 16, i32 1275068685)
  %1 = bitcast i8* %call to i32*
  store i32 0, i32* %1, align 4, !tbaa !2
  %arrayidx1 = getelementptr inbounds i32, i32* %1, i64 1
  store i32 1, i32* %arrayidx1, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds i32, i32* %1, i64 2
  store i32 2, i32* %arrayidx2, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds i32, i32* %1, i64 3
  store i32 3, i32* %arrayidx3, align 4, !tbaa !2
  %2 = load i32, i32* %1, align 4, !tbaa !2
  %3 = load i32, i32* %arrayidx1, align 4, !tbaa !2
  %4 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %call8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %2, i32 %3, i32 %4, i32 3)
  call void @free(i8* %call) #3
  store i32 0, i32* %0
  br label %cato_finalize

cato_finalize:                                    ; preds = %entry.split
  call void @_Z13cato_finalizev()
  %5 = load i32, i32* %0
  ret i32 %5
}
*/
