#ifndef WRAP_MMICTRL_H_INCLUDED
#define WRAP_MMICTRL_H_INCLUDED

#include <cstdint>
#include <map>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace wrap
{

struct error
	: virtual std::runtime_error
{
	error(std::string const &message, std::uint32_t win32_error);

	const std::uint32_t win32_error;
};

enum class init_status
{
	FIRMWARE_LOADED,
	FIRMWARE_ALREADY_LOADED
};

enum class callback_type_type : std::uint8_t
{
	/*
	 * Statusmeldungen für Firmwareübertragung zur Steuerung.
	 * ulParam enthält den Transfer-Handle der entsprechenden Übertragung.
	 * ncrGetTransferState und ncrGetTransferType können im Callback verwendet werden,
	 * um Übertragungsstatus abzufragen.
	 */
	MMI_DOWNLOAD_STATE,
	MMI_DOWNLOAD_PART,
	MMI_DOWNLOAD_COMPLETE,
	MMI_DOWNLOAD_ERROR,
	/*
	 * Statusmeldungen einer laufenden Dateiübertragung zwischen PC und Steuerung.
	 * ulParam enthält den Transfer-Handle der entsprechenden Übertragung.
	 * ncrGetTransferState und ncrGetTransferType können im Callback verwendet werden,
	 * um Übertragungsstatus abzufragen.
	 */
	MMI_TRANSFER_STATE, /* = 4 */
	MMI_TRANSFER_OK,
	MMI_TRANSFER_ERROR,
	MMI_TRANSFER_BREAK,
	/*
	 * Typen für ncrPostMessage
	 * Wenn iRespKey bei ncrPostMessage = -1, dann wird _SENT im Callback geschickt.
	 * Bei einem Fehler in der Übertragung zur Steuerung, wird _NOT_SENT im Callback
	 * geschickt und ulParam = iRespKey.
	 */
	MMI_NCMSG_SENT,
	MMI_NCMSG_NOT_SENT,
	/*
	 * Empfang einer Nachricht der Steuerung, die nicht von der DLL ausgewertet werden kann.
	 * ulParam = Pointer auf MSG_TR-Struktur (Speicher nur innerhalb des Callbacks gültig).
	 */
	MMI_NCMSG_RECEIVED, /* = 10 */
	/*
	 * Übergabe von Fehlermeldungen von Steuerung, Gateway oder DLL.
	 * ulParam = Pointer auf MSG_TR-Struktur (Speicher nur innerhalb des Callbacks gültig).
	 * Fehlermeldungen der Klasse 3 und 4 müssen quittiert werden (Senden von
	 * SB1_FEHLERQUITTING_KUC oder SB1_FEHLERQUITTING_ALLE_KUC an die Steuerung).
	 */
	MMI_ERROR_MSG,
	/*
	 * Zyklischer Aufruf, bei Verwendung von blockierenden Aufrufen (alle 100ms).
	 */
	MMI_CYCLIC_CALL,
	/*
	 * Meldet den Status in ulParam, wenn die Standard-Applikation vom MMI-Gateway
	 * umgeschaltet wird.
	 */
	MMI_DEFAPP_STATE,
	/*
	 * Meldet den Status in ulParam (Pointer auf CAN_TRANSFER_STATE_TR) der laufenden
	 * CANopen SDO-Übertragung.
	 */
	MMI_CAN_TRANSFER_STATE, /* 14 */
	MMI_CAN_TRANSFER_COMPLETE,
	/*
	 * Nachricht konnte nicht von dieser Implementierung verarbeitet werden
	 */
	MMI_UNIMPLEMENTED,
};

enum class transfer_status_type
{
	OK, // Transfer erfolgreich
	BREAK, // Transfer abgebrochen
	FAIL, // Zugriffsfehler, Prüfen der WinAPI Codes
	TIMEOUT, // Transfer-Timeout
	NOINIT, // Firmware-Download wurde noch nicht durchgeführt
	QFULL // Zu viele Transferaufträge gleichzeitig aktiv
};

enum class transfer_block_type
{
	NC_PROGRAMM,
	ONLINE_PROGRAMM,
	WERKZEUG,
	WERKSTUECK,
	ACHSE,
	TECHNOLOGIE,
	MASCHINENKONSTANTEN,
	KONFIGURATION,
	PFELD,
	IO_MODUL_OFFSETS,
	KORREKTUR_3D,
	CAN_IO_OBJEKT,
	CAN_DRIVE_OBJEKT
};

// for blocked variants
struct transfer_exception
	: virtual std::runtime_error
{
	transfer_exception(std::string const &message, transfer_status_type type);

	const transfer_status_type type;
};

struct transfer_exception_error
	: error,
	  transfer_exception
{
	transfer_exception_error(std::string const &message, transfer_status_type type, std::uint32_t win32_error);
};

struct transfer_message
{
	std::uint8_t controlblock0;
	std::uint8_t controlblock1;
	std::uint8_t controlblock2;
	std::uint8_t current_block_number;
	std::uint8_t sender;
	std::uint8_t handle;
	std::vector<std::uint8_t> data;
};

typedef std::function<void(std::uint8_t *)> callback_function_type;

struct mmictrl
{
	virtual ~mmictrl();


	// Schließen und Beenden der Steuerung
	virtual void close() = 0;

	// Zeiger auf Dual-Port-RAM der Steuerung
	//virtual void *get_dpr_address() = 0;

	// Ermitteln, ob Steuerung die Firmware schon erhalten hat
	virtual bool get_init_state() = 0;

	// Asynchrone Funktion, um die Firmware und das SPS-Programm in die Steuerung zu laden
	// Parameter: Name der Konfigurationsdatei für Firmware-Download
	//virtual transfer_type load_firmware(std::string const &config_name) = 0;

	// Synchrone Funktion, um die Firmware und das SPS-Programm in die Steuerung zu laden
	// Parameter: Name der Konfigurationsdatei für Firmware-Download
	virtual init_status load_firmware_blocked(std::string const &config_name) = 0;

	// Verbindungsaufbau zu einer bereits geladenen Steuerung für passive Applikationen, wie
	// Busserver und CodeSys.
	//virtual void connect() = 0;

	// Asynchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme in die
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Header-String für Programmnummern bei DIN-Programmen, Steuerblock 1 für
	// Blockübertragungen
	//virtual transfer_type send_file(std::string const &name, std::string const &header, transfer_block_type type) = 0;

	// Erweiterte asynchrone Funktion, um vorwiegend Online-Programme, ab einem bestimmten Offset
	// in die Steuerung zu übertragen.
	// Parameter: Dateiname, Offset, Header-String, Steuerblock 1
	//virtual transfer_type send_file_ex(std::string const &name, std::uint64_t offset, std::string const &header,
	//                                   transfer_block_type type) = 0;

	// Synchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme in die
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Header-String für Programmnummern bei DIN-Programmen, Steuerblock 1 für
	// Blockübertragungen
	virtual void send_file_blocked(std::string const &name, std::string const &header,
	                               transfer_block_type type) = 0;

	// Asynchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme von der
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Steuerblock 1 für Blockübertragungen
	//virtual transfer_type receive_file(std::string const &name, transfer_block_type type) = 0;

	// Synchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme von der
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Steuerblock 1 für Blockübertragungen
	virtual void receive_file_blocked(std::string const &name, transfer_block_type type) = 0;

	// Synchrone Funktion, um Nachrichten an die Steuerung zu senden
	// Parameter: Nachrichten-Struktur
	virtual void send_message(transfer_message const &message) = 0;

	// Lese die Werte der angegebenen P-Feldindices aus dem Parameterfeld der Steuerung.
	// Parameter: Parameters
	virtual void read_param_array(std::map<std::uint16_t, double> &parameters) = 0;

	// Schreibe die angegebenen P-Feldwerte in das Parameterfeld der Steuerung.
	// Parameter: Parameters
	//virtual bool write_param_array(std::map<std::uint16_t, double> &parameters) = 0;

	// Liest Echtzeit-Daten aus der Steuerung.
	// Parameter: Liste von Ausdrücken, Auswertungen werden gespeichert, Fehlerinformation wird befüllt
	//virtual bool evaluate_expression(std::vector<std::string> const &expressions, std::vector<double> &results,
	//                                 expression_error_info &error_info) = 0;

	// Callback für Steuerungs-Nachrichten
	// Parameter: Aufruf-Typ, Zusatzparameter je nach Wert des Aufruf-Typs
	void set_message_callback(callback_function_type const &function);
	callback_function_type const &get_message_callback() const;

	// Reset CNC
	void reset();

private:
	callback_function_type callback_;
};

struct gateway
{
	// Zugriff auf Konfiguration des Gateways
	// Parameter: Name des Gateways (derzeit nur ein leerer String unterstützt)
	virtual gateway *open(std::string const &name) = 0;

	// Anzahl der konfigurierten Verbindungen
	// Parameter: Anzahl wird befüllt
	virtual std::int32_t get_num_of_conns(std::int32_t number) = 0;
};

}
#endif
