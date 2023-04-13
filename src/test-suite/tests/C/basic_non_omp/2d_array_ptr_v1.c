// RUN: ${CATO_ROOT}/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t_modified.x) %s.reference_output
#include <stdlib.h>

int main()
{
    int** arr = malloc(sizeof(int*) * 2);
    
    arr[0] = malloc(sizeof(int) * 2);
    arr[1] = malloc(sizeof(int) * 2);

    arr[0][0] = 0;
    arr[0][1] = 1;
    arr[1][0] = 2;
    arr[1][1] = 3;

    printf("[%d, %d]\n[%d, %d]\n", arr[0][0], arr[0][1], arr[1][0], arr[1][1]);

    free(arr[0]);
    free(arr[1]);
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
  %1 = bitcast i8* %call to i32**
  %call1 = call i8* @_Z22allocate_shared_memoryli(i64 8, i32 1275068685)
  %2 = bitcast i8* %call1 to i32*
  store i32* %2, i32** %1, align 8, !tbaa !2
  %call2 = call i8* @_Z22allocate_shared_memoryli(i64 8, i32 1275068685)
  %3 = bitcast i8* %call2 to i32*
  %arrayidx3 = getelementptr inbounds i32*, i32** %1, i64 1
  store i32* %3, i32** %arrayidx3, align 8, !tbaa !2
  %4 = load i32*, i32** %1, align 8, !tbaa !2
  store i32 0, i32* %4, align 4, !tbaa !6
  %5 = load i32*, i32** %1, align 8, !tbaa !2
  %arrayidx7 = getelementptr inbounds i32, i32* %5, i64 1
  store i32 1, i32* %arrayidx7, align 4, !tbaa !6
  %6 = load i32*, i32** %arrayidx3, align 8, !tbaa !2
  store i32 2, i32* %6, align 4, !tbaa !6
  %7 = load i32*, i32** %arrayidx3, align 8, !tbaa !2
  %arrayidx11 = getelementptr inbounds i32, i32* %7, i64 1
  store i32 3, i32* %arrayidx11, align 4, !tbaa !6
  %8 = load i32*, i32** %1, align 8, !tbaa !2
  %9 = load i32, i32* %8, align 4, !tbaa !6
  %arrayidx15 = getelementptr inbounds i32, i32* %8, i64 1
  %10 = load i32, i32* %arrayidx15, align 4, !tbaa !6
  %11 = load i32*, i32** %arrayidx3, align 8, !tbaa !2
  %12 = load i32, i32* %11, align 4, !tbaa !6
  %arrayidx19 = getelementptr inbounds i32, i32* %11, i64 1
  %13 = load i32, i32* %arrayidx19, align 4, !tbaa !6
  %call20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i32 0, i32 0), i32 %9, i32 %10, i32 %12, i32 %13)
  %14 = load i32*, i32** %1, align 8, !tbaa !2
  %15 = bitcast i32* %14 to i8*
  call void @free(i8* %15) #3
  %16 = load i32*, i32** %arrayidx3, align 8, !tbaa !2
  %17 = bitcast i32* %16 to i8*
  call void @free(i8* %17) #3
  call void @free(i8* %call) #3
  store i32 0, i32* %0
  br label %cato_finalize

cato_finalize:                                    ; preds = %entry.split
  call void @_Z13cato_finalizev()
  %18 = load i32, i32* %0
  ret i32 %18
}
*/
