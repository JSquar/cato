// PASS: *
// RUN: ${DAS_TOOL_ROOT}/src/scripts/cexecute_pass.py %s -o %t
// RUN: diff <(mpirun -np 4 %t) %s.reference_output
#include <stdlib.h>

int main()
{
    int* data = malloc( sizeof(int) * 4);
    int** arr = malloc(sizeof(int*)*2);

    arr[0] = data;
    arr[1] = data + 2;

    arr[0][0] = 0;
    arr[0][1] = 1;
    arr[1][0] = 2;
    arr[1][1] = 3;

    printf("[%d, %d],\n[%d, %d]\n",arr[0][0], arr[0][1], arr[1][0], arr[1][1]);

    free(data);
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
  %call1 = call i8* @_Z22allocate_shared_memoryli(i64 16, i32 1275068685)
  %2 = bitcast i8* %call1 to i32**
  store i32* %1, i32** %2, align 8, !tbaa !2
  %add.ptr = getelementptr inbounds i32, i32* %1, i64 2
  %arrayidx2 = getelementptr inbounds i32*, i32** %2, i64 1
  store i32* %add.ptr, i32** %arrayidx2, align 8, !tbaa !2
  %3 = load i32*, i32** %2, align 8, !tbaa !2
  store i32 0, i32* %3, align 4, !tbaa !6
  %4 = load i32*, i32** %2, align 8, !tbaa !2
  %arrayidx6 = getelementptr inbounds i32, i32* %4, i64 1
  store i32 1, i32* %arrayidx6, align 4, !tbaa !6
  %5 = load i32*, i32** %arrayidx2, align 8, !tbaa !2
  store i32 2, i32* %5, align 4, !tbaa !6
  %6 = load i32*, i32** %arrayidx2, align 8, !tbaa !2
  %arrayidx10 = getelementptr inbounds i32, i32* %6, i64 1
  store i32 3, i32* %arrayidx10, align 4, !tbaa !6
  %7 = load i32*, i32** %2, align 8, !tbaa !2
  %8 = load i32, i32* %7, align 4, !tbaa !6
  %arrayidx14 = getelementptr inbounds i32, i32* %7, i64 1
  %9 = load i32, i32* %arrayidx14, align 4, !tbaa !6
  %10 = load i32*, i32** %arrayidx2, align 8, !tbaa !2
  %11 = load i32, i32* %10, align 4, !tbaa !6
  %arrayidx18 = getelementptr inbounds i32, i32* %10, i64 1
  %12 = load i32, i32* %arrayidx18, align 4, !tbaa !6
  %call19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0), i32 %8, i32 %9, i32 %11, i32 %12)
  call void @free(i8* %call) #3
  call void @free(i8* %call1) #3
  store i32 0, i32* %0
  br label %cato_finalize

cato_finalize:                                    ; preds = %entry.split
  call void @_Z13cato_finalizev()
  %13 = load i32, i32* %0
  ret i32 %13
}
*/
