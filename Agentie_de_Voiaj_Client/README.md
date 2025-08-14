# Agentie de Voiaj - Client Qt ✈️

Client Qt pentru aplicația de agenție de voiaj cu arhitectură îmbunătățită și funcționalități complete.

## 🎯 Funcționalități Implementate

### 🔐 **Autentificare și Gestionare Utilizatori**
- Dialog de login/register cu validări robuste în română
- Autentificare securizată cu hash-uri de parolă
- Înregistrare utilizatori noi cu validare completă de câmpuri
- Gestionarea automată a sesiunii și logout

### 🏗️ **Arhitectură Profesională**
- **Configurații centralizate** în `config.h` (rețea, UI, validări, mesaje)
- **Sistem de utilitare** în `utils.h/cpp` (logging, validări, JSON, crypto)
- **Network manager îmbunătățit** cu retry logic și timeout-uri configurabile
- **Gestionare erori structurată** cu coduri de eroare și timestamp-uri

### 🌐 **Comunicare cu Serverul**
- Protocol JSON cu terminatori `\r\n` conform serverului
- Compatibilitate cu formatul câmpurilor serverului (`User_ID`/`username`)
- Keep-alive pentru menținerea conexiunii
- Logging complet al cereri/răspunsuri pentru debugging
- Reconnectare automată și gestionarea erorilor de rețea

### 🏠 **Interfață Utilizator Îmbunătățită**
- **Flux de ferestre optimizat**: Login dialog → Main window doar după autentificare
- **Tab "Oferte"**: Vizualizare și rezervare oferte cu validări
- **Tab "Căutare"**: Căutare avansată cu filtre (destinație, preț, date)
- **Tab "Rezervările Mele"**: Gestionarea completă a rezervărilor
- **Tab "Profilul Meu"**: Actualizare date personale cu validări

### 🎯 **Operațiuni de Booking Robuste**
- Rezervare oferte cu validarea numărului de persoane (1-10)
- Anulare rezervări cu confirmare
- Actualizare profil cu validări în timp real
- Verificări de business logic (prețuri, disponibilitate)

### 🎨 **UI/UX Profesional**
- Design modern cu scheme de culori configurabile
- Stilizare CSS personalizată pentru toate componentele
- Tabele sortabile cu redimensionare automată
- Status bar cu indicatori de conexiune și utilizator
- Meniu și toolbar complete cu acțiuni contextualizate
- Mesaje de progres și notificări user-friendly
- Suport pentru teme (default, dark mode viitor)

## 📁 Fișiere și Organizare

### **Configurații și Utilitare**
- `config.h` - Configurații centralizate pentru întreaga aplicație
- `utils.h/cpp` - Sistem complet de utilitare organizat în namespace-uri

### **Componente Principale**
- `network_manager.h/cpp` - Manager de comunicație cu server îmbunătățit
- `login_dialog.h/cpp` - Dialog de autentificare cu validări complete
- `main_window.h/cpp` - Fereastra principală cu gestionare ciclul de viață
- `main.cpp` - Entry point optimizat pentru fluxul de ferestre

### **Organizare Visual Studio**
- **Configuration** folder - Fișiere de configurație
- **Utilities** folder - Utilitare și helperi
- **Source Files** - Cod principal al aplicației
- **Header Files** - Qt headers și MOC files

## 🛠️ Funcții Tehnice Avansate

### **Logging și Debugging**
- Logging multicanal (consolă + fișier)
- Niveluri configurabile de log (Debug, Info, Warning, Error, Critical)
- Network request/response logging pentru debugging
- Timestamp-uri și categorii pentru toate log-urile

### **Validări și Securitate**
- Validări de câmpuri cu regex patterns configurabile
- Mesaje de eroare localizate în română
- Hash-uri de parole cu salt pentru securitate
- Validări de business logic (prețuri, limites persoane)

### **Gestionare Conexiuni**
- Timeout-uri configurabile pentru conexiune (30s default)
- Retry logic cu încercări multiple
- Keep-alive la 60s pentru menținerea conexiunii
- Graceful disconnect la închiderea aplicației

## 🔧 Protocol de Comunicare

### Cereri Client → Server
```json
{
  "type": "AUTH",
  "username": "utilizator123",
  "password": "parola123"
}
```

### Răspunsuri Server → Client
```json
{
  "success": true,
  "message": "Autentificare reușită",
  "data": {
    "User_ID": 1,
    "Username": "utilizator123",
    "Email": "user@example.com",
    "First_Name": "Ion",
    "Last_Name": "Popescu",
    "Phone": "+40712345678"
  }
}
```

### Operațiuni Suportate
- `AUTH` - Autentificare utilizator
- `REGISTER` - Înregistrare utilizator nou
- `GET_DESTINATIONS` - Obținere destinații
- `GET_OFFERS` - Obținere oferte disponibile
- `SEARCH_OFFERS` - Căutare cu filtre
- `BOOK_OFFER` - Rezervare ofertă
- `GET_USER_RESERVATIONS` - Rezervările utilizatorului
- `CANCEL_RESERVATION` - Anulare rezervare
- `GET_USER_INFO` - Informații profil
- `UPDATE_USER_INFO` - Actualizare profil
- `KEEPALIVE` - Menținere conexiune

## 📋 Cerințe și Dependențe

### **Build Requirements**
- Visual Studio 2019/2022
- Qt 6.9.1 cu modulele: Core, GUI, Widgets, Network
- vcpkg pentru managementul dependențelor
- MSVC 2022 toolchain

### **Runtime Dependencies**
- Server Agentie de Voiaj rulând pe port 8080
- SQL Server pentru baza de date (gestionată de server)

## 🚀 Compilare și Rulare

### **Compilare**
```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" Agentie_de_Voiaj.sln -p:Configuration=Debug -p:Platform=x64
```

### **Setup DLL-uri și Plugin-uri Qt (OBLIGATORIU)**
După compilare, rulează scriptul pentru copierea DLL-urilor și plugin-urilor Qt:
```cmd
copy_qt_dlls.bat
```
Sau manual pentru Debug:
```cmd
REM DLL-uri principale
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Cored.dll" "x64\Debug\"
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Guid.dll" "x64\Debug\" 
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Widgetsd.dll" "x64\Debug\"
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Networkd.dll" "x64\Debug\"

REM Platform plugins (necesare pentru GUI)
mkdir "x64\Debug\platforms"
copy "C:\Qt\6.9.1\msvc2022_64\plugins\platforms\qwindowsd.dll" "x64\Debug\platforms\"
copy "C:\Qt\6.9.1\msvc2022_64\plugins\platforms\qminimald.dll" "x64\Debug\platforms\"

REM Image format plugins (suport imagini)
mkdir "x64\Debug\imageformats"
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qjpegd.dll" "x64\Debug\imageformats\"
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qgifd.dll" "x64\Debug\imageformats\"
copy "C:\Qt\6.9.1\msvc2022_64\plugins\imageformats\qsvgd.dll" "x64\Debug\imageformats\"
```

### **Rulare**
1. **Pornește serverul**:
   ```
   cd x64\Debug
   Agentie_de_Voiaj_Server.exe
   ```

2. **Pornește clientul**:
   ```
   cd x64\Debug
   Agentie_de_Voiaj_Client.exe
   ```

## 💻 Utilizare și Fluxul de Lucru

### **Prima Utilizare**
1. Se deschide doar dialogul de login
2. Înregistrarea utilizatorului nou sau autentificare
3. Fereastra principală apare doar după login cu succes

### **Navigare**
- **Oferte**: Răsfoiește și rezervă oferte de călătorie
- **Căutare**: Filtrează oferte după destinație, preț, date
- **Rezervările Mele**: Gestionează rezervările existente
- **Profilul Meu**: Actualizează datele personale

### **Deconectare**
- Logout ascunde fereastra principală și redeschide login-ul
- Exit application închide graceful cu deconectare automată

## 🐛 Debugging și Troubleshooting

### **Log Files**
- Log-urile se salvează în `%AppData%\Agentie de Voiaj\logs\app.log`
- Network logs cu detalii complete pentru debugging

### **Probleme Comune**
- **"Nu poate rula aplicația - lipsesc DLL-uri Qt"**: Rulează `copy_qt_dlls.bat` sau copiază manual DLL-urile Qt6Cored.dll, Qt6Guid.dll, Qt6Widgetsd.dll, Qt6Networkd.dll în directorul x64\Debug
- **"No Qt platform plugin could be initialized"**: Creează directorul `x64\Debug\platforms` și copiază qwindowsd.dll, qminimald.dll din `C:\Qt\6.9.1\msvc2022_64\plugins\platforms\`
- **Server neconectat**: Verifică că serverul rulează pe port 8080
- **Autentificare eșuată**: Verifică username/password în logs
- **Timeout conectiune**: Configurabil în `Config::Network::CONNECTION_TIMEOUT_MS`

## 🎨 Personalizare

### **Configurări UI**
```cpp
namespace Config::UI {
    const QString PRIMARY_COLOR = "#4CAF50";     // Verde principal
    const QString SECONDARY_COLOR = "#2196F3";   // Albastru secundar
    const QString SUCCESS_COLOR = "#4CAF50";     // Verde succes
    const QString ERROR_COLOR = "#f44336";       // Roșu erori
}
```

### **Configurări Network**
```cpp
namespace Config::Network {
    const QString DEFAULT_SERVER_HOST = "localhost";
    constexpr int DEFAULT_SERVER_PORT = 8080;
    constexpr int CONNECTION_TIMEOUT_MS = 30000;  // 30 secunde
}
```

## 📈 Îmbunătățiri Făcute

✅ **Repararea dublelor ferestre** - Doar login dialog la început  
✅ **Protocol JSON corectat** - Terminatori `\r\n` pentru server  
✅ **Compatibilitate câmpuri** - Suport pentru `User_ID`/`username`  
✅ **Logging sistematic** - Debug complet pentru rețea și aplicație  
✅ **Validări robuste** - Mesaje în română și verificări complete  
✅ **Arhitectură modulară** - Config centralizat și utilitare reutilizabile  
✅ **Gestionare conexiuni** - Timeout-uri, retry logic, graceful cleanup  
✅ **Window lifecycle** - Fluxul corect de afișare/ascundere ferestre

Aplicația este acum robustă, profesională și gata pentru producție! 🎉