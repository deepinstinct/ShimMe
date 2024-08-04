# ShimMe
Tools from the DEFCON 32 talk "SHIM me what you got - Manipulating Shim and Office for Code Injection"

Office Injector - Invokes an RPC method in OfficeClickToRun service that will inject a DLL into a suspended process running as NT AUTHORITY\SYSTEM launched by the task scheduler service, thus achieving privilege escalation from administrator to SYSTEM.

Shim Injector - Writes an undocumented shim data structure into the memory of another process that causes apphelp.dll to apply the “Inject Dll” fix on the process without registering a new SDB file on the system, or even writing such file to disk.

## Credits

* [David Shandalov](https://twitter.com/DavidShandalov)
* [Ron Ben Yizhak](https://twitter.com/RonB_Y)


## References

* https://learn.microsoft.com/en-us/windows/win32/fileio/opportunistic-locks
* https://learn.microsoft.com/en-us/windows/win32/devnotes/application-compatibility-database
* https://www.blackhat.com/docs/eu-15/materials/eu-15-Pierce-Defending-Against-Malicious-Application-Compatibility-Shims-wp.pdf
* https://www.blackhat.com/docs/asia-14/materials/Erickson/WP-Asia-14-Erickson-Persist-It-Using-And-Abusing-Microsofts-Fix-It-Patches.pdf