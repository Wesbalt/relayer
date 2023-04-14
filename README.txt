
 * Projektet kompileras med GCC (version 12.2.0, target x86_64-w64-mingw32)
   och körs på Windows 10 64-bit. Andra OS stöds ej.

 * Projektet inkluderar en testsvit som testar ordentligt men är primitivt
   skriven.

 * Det finns inga formella doc-strängar men jag har ändå kommenterat så att
   man kan enkelt hänga med koden.

 * En BAT och MAKEFILE används för att enkelt använda projektet. Följande
   kommandon finns tillgängliga:
       run_server  - kör servern
       make build  - bygg klienten
       make run    - kör klienten
       make tbuild - bygg tester
       make trun   - kör tester
       make clean  - rensa EXE-filerna
