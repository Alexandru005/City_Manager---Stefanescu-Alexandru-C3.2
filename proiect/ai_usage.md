# Documentație Utilizare AI - Proiect City Manager

## Tool Folosit
Claude (Anthropic) - claude.ai

---

## Faza 1 - Funcția Filter

### Ce Am Cerut
Am descris structura `report_t` lui Claude și am cerut să genereze două funcții:
1. `parse_condition(const char *input, char *field, char *op, char *value)`
   care împarte un string de forma `field:operator:value` în cele trei părți
2. `match_condition(report_t *r, const char *field, const char *op, const char *value)`
   care returnează 1 dacă un raport satisface condiția și 0 altfel

Structura pe care am descris-o:
```c
typedef struct Report {
    int report_id;
    char inspector_name[50];
    gps_t gps_coordinates;
    char issue_category[25];
    int severity_level;
    time_t timestamp;
    char description[128];
} report_t;
```

### Ce A Generat
Claude a generat ambele funcții. `parse_condition` folosește `strtok()` pentru
a împărți string-ul de intrare după delimitatorul `:` în field, operator și value.
`match_condition` compară câmpurile raportului cu condiția folosind `strcmp()`
pentru string-uri și `atoi()`/`atol()` pentru conversii numerice.

### Ce Am Modificat
- Am integrat funcțiile în codul meu existent
- Am scris eu logica funcției `filter`: deschiderea fișierului reports.dat,
  citirea rapoartelor unul câte unul cu `read()`, apelarea `parse_condition()`
  și `match_condition()` pentru fiecare raport, și afișarea rapoartelor care
  satisfac toate condițiile

### Ce Am Învățat
- Cum funcționează `strtok()` pentru împărțirea string-urilor după un delimiter
- Cum se convertesc string-uri la întregi cu `atoi()` și `atol()`
- Importanța verificării codului generat de AI linie cu linie înainte de utilizare
- AI-ul generează structura corectă dar integrarea în codul existent
  necesită înțelegerea întregului proiect

