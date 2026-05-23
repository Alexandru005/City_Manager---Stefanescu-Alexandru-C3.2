# Ghid de Utilizare și Rulare - Proiect SO

Acest document cuprinde toate comenzile disponibile și pașii necesari pentru compilarea, rularea și testarea sistemului de infrastructură urbană.

---

## I. Utilitarul `./city_manager`

Toate comenzile destinate `city_manager` se rulează direct din terminal și necesită transmiterea explicită a rolului (`--role manager` sau `--role inspector`) și a numelui de utilizator (`--user <nume>`).

* **Adăugare raport nou / inițializare district:**
  `./city_manager --role manager --user alin --add Arad`
  `./city_manager --role inspector --user vasile --add Timisoara`

* **Listare rapoarte dintr-un district:**
  `./city_manager --role inspector --user vasile --list Arad`

* **Vizualizare detalii raport specific (după ID):**
  `./city_manager --role inspector --user vasile --view Arad 1`

* **Ștergere raport din fișierul binar (Doar Manager):**
  `./city_manager --role manager --user alin --remove_report Arad 1`

* **Actualizare prag de severitate (Doar Manager):**
  `./city_manager --role manager --user alin --update_threshold Arad 2`

* **Filtrare avansată a rapoartelor:**
  `./city_manager --role inspector --user vasile --filter Arad severity:>=:2 category:==:road`

* **Ștergere completă district (Doar Manager):**
  `./city_manager --role manager --user alin --remove_district Arad`

---

## II. Interfața Centralizată `city_hub`

Această componentă rulează ca o consolă interactivă. Mai întâi se lansează executabilul din terminal, apoi comenzile se scriu în promptul `> `.

Lansarea programului:
`./city_hub`

Comenzi în interiorul interfeței:
* **Pornire Monitor în fundal:** `start_monitor`
* **Calculare scoruri și workload inspectori:** `calculate_scores Arad Timisoara`
* **Oprire și părăsire interfață:** `exit`
