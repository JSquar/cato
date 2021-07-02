; ModuleID = 'array_pointer_var.cpp'
source_filename = "array_pointer_var.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%ident_t = type { i32, i32, i32, i32, i8* }

$__clang_call_terminate = comdat any

@.str = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@0 = private unnamed_addr constant %ident_t { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str, i32 0, i32 0) }, align 8
@.str.1 = private unnamed_addr constant [34 x i8] c"[%d, %d, %d, %d, %d, %d, %d, %d]\0A\00", align 1

; Function Attrs: norecurse uwtable
define i32 @main() #0 {
entry:
  %a = alloca i32*, align 8
  %0 = bitcast i32** %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #8
  %call = call i8* @_Znam(i64 32) #9
  %1 = bitcast i8* %call to i32*
  store i32* %1, i32** %a, align 8, !tbaa !2
  call void (%ident_t*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(%ident_t* @0, i32 1, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**)* @.omp_outlined. to void (i32*, i32*, ...)*), i32** %a)
  %2 = load i32*, i32** %a, align 8, !tbaa !2
  %3 = load i32, i32* %2, align 4, !tbaa !6
  %arrayidx1 = getelementptr inbounds i32, i32* %2, i64 1
  %4 = load i32, i32* %arrayidx1, align 4, !tbaa !6
  %arrayidx2 = getelementptr inbounds i32, i32* %2, i64 2
  %5 = load i32, i32* %arrayidx2, align 4, !tbaa !6
  %arrayidx3 = getelementptr inbounds i32, i32* %2, i64 3
  %6 = load i32, i32* %arrayidx3, align 4, !tbaa !6
  %arrayidx4 = getelementptr inbounds i32, i32* %2, i64 4
  %7 = load i32, i32* %arrayidx4, align 4, !tbaa !6
  %arrayidx5 = getelementptr inbounds i32, i32* %2, i64 5
  %8 = load i32, i32* %arrayidx5, align 4, !tbaa !6
  %arrayidx6 = getelementptr inbounds i32, i32* %2, i64 6
  %9 = load i32, i32* %arrayidx6, align 4, !tbaa !6
  %arrayidx7 = getelementptr inbounds i32, i32* %2, i64 7
  %10 = load i32, i32* %arrayidx7, align 4, !tbaa !6
  %call8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.1, i32 0, i32 0), i32 %3, i32 %4, i32 %5, i32 %6, i32 %7, i32 %8, i32 %9, i32 %10)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %0) #8
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind readnone speculatable
declare { i64, i1 } @llvm.umul.with.overflow.i64(i64, i64) #2

; Function Attrs: nobuiltin
declare noalias nonnull i8* @_Znam(i64) #3

; Function Attrs: nounwind uwtable
define internal void @.omp_outlined.(i32* noalias %.global_tid., i32* noalias %.bound_tid., i32** dereferenceable(8) %a) #4 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = invoke i32 @omp_get_thread_num()
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %0 = load i32*, i32** %a, align 8, !tbaa !2
  %idxprom = sext i32 %call to i64
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %idxprom
  store i32 %call, i32* %arrayidx, align 4, !tbaa !6
  %1 = load i32*, i32** %a, align 8, !tbaa !2
  %add = add nsw i32 %call, 4
  %idxprom1 = sext i32 %add to i64
  %arrayidx2 = getelementptr inbounds i32, i32* %1, i64 %idxprom1
  store i32 42, i32* %arrayidx2, align 4, !tbaa !6
  ret void

lpad:                                             ; preds = %entry
  %2 = landingpad { i8*, i32 }
          catch i8* null
  %3 = extractvalue { i8*, i32 } %2, 0
  call void @__clang_call_terminate(i8* %3) #10
  unreachable
}

declare i32 @omp_get_thread_num() #5

declare i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) #6 comdat {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #8
  call void @_ZSt9terminatev() #10
  unreachable
}

declare i8* @__cxa_begin_catch(i8*)

declare void @_ZSt9terminatev()

declare void @__kmpc_fork_call(%ident_t*, i32, void (i32*, i32*, ...)*, ...)

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) #7

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noinline noreturn nounwind }
attributes #7 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { nounwind }
attributes #9 = { builtin }
attributes #10 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://llvm.org/git/clang.git e2686b2cbccd63ecf0f1f346b7230bf4aa7d6a7f) (https://llvm.org/git/llvm.git 6cf02b953eb786809e75e20b9f556e4791b2eabe)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"any pointer", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
