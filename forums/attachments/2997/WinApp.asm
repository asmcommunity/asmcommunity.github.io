; =================================================================================================
; Title:   WinApp.asm
; Author:  G. Friedrich
; Version: 1.0.0
; Purpose: ObjAsm32 compilation file for WinApp object.
; Version: Version 1.0.0, August 2008
;            - First release.
; =================================================================================================

%include @Environ(OA32_PATH)\\Code\\Macros\\Model.inc
SysSetup OOP_OBJECT_LIB

% include &MacPath&WinHelpers.inc

;Add here all files that build the inheritance path and referenced objects
include Primer.inc
include Streamable.inc
include Collection.inc
include DwordCollection.inc
include WinPrimer.inc

;Add here the file that defines the object(s) to be included in the library
MakeObjects WinApp

end
