#include <stdlib.h>

int main()
{
    int* data = malloc(sizeof(int));
    int** arr = malloc(sizeof(int*));

    arr[0] = data;
    arr[0][0] = 42;

    printf("%d\n", arr[0][0]);
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
  %call = call i8* @_Z22allocate_shared_memoryli(i64 4, i32 1275068685)
  %1 = bitcast i8* %call to i32*
  %call1 = call i8* @_Z22allocate_shared_memoryli(i64 8, i32 1275068685)
  %2 = bitcast i8* %call1 to i32**
  store i32* %1, i32** %2, align 8, !tbaa !2
  store i32 42, i32* %1, align 4, !tbaa !6
  %3 = load i32*, i32** %2, align 8, !tbaa !2
  %4 = load i32, i32* %3, align 4, !tbaa !6
  %call6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %4)
  call void @free(i8* %call) #3
  call void @free(i8* %call1) #3
  store i32 0, i32* %0
  br label %cato_finalize

cato_finalize:                                    ; preds = %entry.split
  call void @_Z13cato_finalizev()
  %5 = load i32, i32* %0
  ret i32 %5
}
*/
