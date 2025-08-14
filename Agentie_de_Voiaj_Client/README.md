# Agentie de Voiaj - Client Qt

Client Qt pentru aplicația de agenție de voiaj.

## Funcționalități implementate

### 🔐 **Autentificare și înregistrare**
- Dialog de login/register cu validări
- Suport pentru autentificare cu server
- Înregistrare utilizatori noi

### 🌐 **Comunicare cu serverul**
- Network manager cu protocoul JSON complet
- Conexiune TCP la server (localhost:8080)
- Keep-alive pentru menținerea conexiunii
- Gestionarea erorilor de rețea

### 🏠 **Interfață principală**
- **Tab "Oferte"**: Vizualizare și rezervare oferte
- **Tab "Căutare"**: Căutare filtrată de oferte
- **Tab "Rezervările Mele"**: Gestionarea rezervărilor
- **Tab "Profilul Meu"**: Actualizare date personale

### 🎯 **Operațiuni de booking**
- Rezervarea ofertelor cu specificarea numărului de persoane
- Anularea rezervărilor existente
- Actualizarea datelor personale

### 🎨 **UI/UX**
- Interfață modernă cu stilizare CSS
- Tabele sortabile și redimensionabile
- Status bar cu starea conexiunii și utilizatorului
- Meniu și toolbar complete
- Notificări și mesaje de eroare

## Fișiere principale

- `network_manager.h/cpp` - Comunicare cu serverul
- `login_dialog.h/cpp` - Dialog de autentificare
- `main_window.h/cpp` - Fereastra principală
- `main.cpp` - Entry point

## Dependințe

- Qt 6.9.1 (Core, GUI, Widgets, Network)
- MSVC 2022
- JSON support pentru comunicarea cu serverul

## Compilare

1. Deschide `Agentie_de_Voiaj.sln` în Visual Studio
2. Asigură-te că Qt este configurat corect
3. Build solution în Debug/Release

## Utilizare

1. Pornește serverul (`Agentie_de_Voiaj_Server.exe`)
2. Pornește clientul (`Agentie_de_Voiaj_Client.exe`)
3. Autentifică-te sau înregistrează-te
4. Explorează ofertele și fă rezervări!