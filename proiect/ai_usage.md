# AI_usage-ALL-phases.md

**Tool folosit:** Google Gemini
**Proiect:** SO/SO1/OS Project – City Infrastructure Issue Reporting

---

## Faza 1: File Systems & Filtrare

**Scopul utilizării AI:** Generarea funcțiilor ajutătoare pentru comanda `filter` (`parse_condition` și `match_condition`), conform permisiunii explicite din cerința proiectului.

**Prompturi oferite:**
1. "Am o aplicație în C care gestionează rapoarte de infrastructură. Structura mea de date conține: `int severity_level`, `char issue_category[25]`, `char inspector_name[50]`, `time_t timestamp`. Generează o funcție `int parse_condition(const char *input, char *field, char *op, char *value)` care împarte un string de tipul 'field:operator:value' în cele 3 componente."
2. "Generează funcția `int match_condition(report_t *r, const char *field, const char *op, const char *value)` care ia aceste 3 componente și returnează 1 dacă structura respectă condiția sau 0 în caz contrar. Gestionează corect conversiile pentru întregi și stringuri."

**Ce a fost generat și ce am modificat:**
* AI-ul a propus inițial utilizarea funcției `strtok` direct pe parametrul `input` în `parse_condition`.
* **Modificare:** Am adăugat o copie a stringului (`char copy[256]; strncpy(copy, input...)`) înainte de a apela `strtok`. Am făcut acest lucru deoarece `strtok` modifică stringul original punând caractere `\0`, iar eu primeam argumentul ca `const char*` din linia de comandă.
* În `match_condition`, AI-ul a generat o structură de bază cu mai multe instrucțiuni `if`. Am verificat ca funcția `atoi()` să fie folosită pentru severitate și conversia corectă `(time_t)atol(value)` pentru timestamp.

**Ce am învățat:**
* Am înțeles cum funcționează `strtok` și de ce este periculos să îl folosești pe stringuri constante.
* Am exersat maparea tipurilor de date din stringuri primite în linia de comandă (char*) în tipurile specifice structurii mele din C pentru a efectua evaluări logice.

---

## Faza 2: Procese și Semnale

**Scopul utilizării AI:** Înțelegerea corectă a utilizării semnalelor între două procese distincte (manager și monitor) și a rulării de comenzi externe cu `execlp`.

**Prompturi oferite:**
1. "Cum trimit un semnal `SIGUSR1` dintr-un proces în altul în C, dacă am salvat PID-ul procesului destinație într-un fișier text?"
2. "Cum creez un proces copil în C pentru a rula comanda `rm -rf` pentru a șterge un director?"

**Ce a fost generat și ce am modificat:**
* AI-ul mi-a arătat cum să folosesc `fork()` combinat cu `execlp()`, dar a inclus și funcția interzisă `signal()` pentru prinderea semnalelor în monitor.
* **Modificare:** Am înlocuit implementarea cu `sigaction()`, conform cerințelor stricte ale proiectului. Am setat corect `sa_handler` și `sigemptyset(&sa_mask)`. De asemenea, am integrat citirea PID-ului din fișierul `.monitor_pid` în funcția mea `add`, adăugând `kill(monitor_pid, SIGUSR1)` și tratând cazurile în care monitorul nu este pornit.

**Ce am învățat:**
* Am învățat diferența esențială dintre `signal()` (care este o funcție moștenită și mai puțin sigură) și `sigaction()` (care oferă un control robust asupra blocării altor semnale în timpul execuției handler-ului).
* Am văzut practic cum comunică procesele asincron prin sistemul de operare.

---

## Faza 3: Pipes și Redirects (dup2)

**Scopul utilizării AI:** Implementarea redirectării output-ului standard al unui proces executabil extern (scorer) către procesul părinte (hub) folosind pipe-uri.

**Prompturi oferite:**
1. "Cum folosesc `pipe` și `dup2` în C pentru a captura ce afișează la `stdout` un proces copil creat cu `fork` și `execlp`?"
2. "Cum mă asigur că procesul părinte (`city_hub`) citește tot output-ul unui monitor care rulează în fundal fără să se blocheze interfața?"

**Ce a fost generat și ce am modificat:**
* Asistentul a oferit șablonul standard pentru crearea unui pipe (`int pfd[2]; pipe(pfd);`), urmat de crearea procesului cu `fork`.
* **Modificare:** Am adaptat logica în bucla `while(district != NULL)` din comanda `calculate_scores` pentru a spana mai multe procese `scorer` secvențial. Am fost foarte atent să închid capetele pipe-ului care nu sunt folosite: `close(pfd[0])` în copil înainte de `dup2`, și `close(pfd[1])` în părinte înainte de `read`.

**Ce am învățat:**
* Am înțeles că un pipe are două capete unidirecționale (0 pentru citire, 1 pentru scriere).
* Cea mai valoroasă lecție a fost că, dacă procesul părinte nu închide capătul de scriere al pipe-ului (`close(pfd[1])`), funcția `read()` va bloca programul la nesfârșit, așteptând date (Deoarece semnalul EOF nu este trimis).
* Am înțeles că funcția `dup2(pfd[1], STDOUT_FILENO)` schimbă destinația funcției `printf` din copil direct în țeava pe care o citește părintele.