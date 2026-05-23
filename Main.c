//
// Copyright (C) C0000374
//
// Крохотный лаунчер для Minecraft beta 1.7.3

#include <NRTL.h>

#pragma warning(disable:4005)

#define CDECL	__cdecl
#define STDCALL __stdcall

PWSTR BlpErrorMessageBuffer;

FORCEINLINE
VOID
x86ZeroMemory(
    OUT PVOID Buffer,
    IN UINT32 BufferSize
    )
{
    __asm {
              push  edi
              mov   edi, Buffer
              mov   ecx, BufferSize
              xor   al, al
        rep   stosb
              pop   edi
    }
}

BOOLEAN
STDCALL
BlpAllocationFailHandler(
    IN UINT32 BlockSize,
    IN PVOID Block OPTIONAL
    )
{
    Nrtl_snwprintf(BlpErrorMessageBuffer, 1024, L"Не удалось выделить 0x%08X байт кучи", BlockSize);
    MessageBoxW(GetFocus(), BlpErrorMessageBuffer, L"BetaLauncher", MB_OK | MB_ICONERROR);
    RtlExitUserProcess(STATUS_NO_MEMORY);
}

VOID
CDECL
WinMainCRTStartup(
    VOID
    )
{
    NTSTATUS Status;
    PWSTR* LauncherCommandLine;
    UINT32 ArgumentsCount;
    PWSTR JavaWCommandLine;
    BOOL b32Status;
    STARTUPINFOW StartupInformation;
    PROCESS_INFORMATION ProcessInformation;

    Status = STATUS_SUCCESS;
    JavaWCommandLine = NULL;

    BlpErrorMessageBuffer = NrtlAllocateHeap(0, 1024);
    NrtlSetAllocFailHandler(&BlpAllocationFailHandler);

    LauncherCommandLine = CommandLineToArgvW(NtCurrentPeb()->ProcessParameters->CommandLine.Buffer, &ArgumentsCount);
    if (ArgumentsCount < 3) {

        MessageBoxW(
            GetFocus(),
            L"Использование:\r\nbl.exe <jar> <username>",
            L"BetaLauncher",
            MB_OK | MB_ICONINFORMATION
        );
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    JavaWCommandLine = NrtlAllocateHeap(0, 1024);
    Nrtl_snwprintf(
        JavaWCommandLine,
        1024,
        L"JavaW.exe -Djava.library.path=Lib -cp LibJava\\JInput-2.0.5.jar;LibJava\\JUtils-1.0.0.jar;LibJava\\LWJGL-2.9.0.jar;LibJava\\LWJGL_Util-2.9.0.jar;%s -Xmx4G net.minecraft.client.Minecraft %s",
        LauncherCommandLine[1],
        LauncherCommandLine[2]
    );

    x86ZeroMemory(&StartupInformation, sizeof(STARTUPINFOW));
    StartupInformation.cb = sizeof(STARTUPINFOW);

    b32Status = CreateProcessW(
        NULL,
        JavaWCommandLine,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &StartupInformation,
        &ProcessInformation
    );
    if (!b32Status) {

        MessageBoxW(
            GetFocus(),
            L"Не удалось запустить JavaW!",
            L"BetaLauncher",
            MB_OK | MB_ICONERROR
        );
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    NtClose(ProcessInformation.hProcess);
    NtClose(ProcessInformation.hThread);

Cleanup:
    LocalFree(LauncherCommandLine);
    if (JavaWCommandLine != NULL) NrtlFreeHeap(JavaWCommandLine);
    RtlExitUserProcess(Status);
}