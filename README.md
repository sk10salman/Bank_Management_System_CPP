# Bank_Management_System_CPP(C++)
This is a Bank Management (Database Management) Project in C++ language created using various concepts of Object-oriented Programming.



script

$url = "https://example.com"  # URL jise hit karna hai
$timeout = 5  # Timeout (seconds) connection check ke liye

try {
    $request = Invoke-WebRequest -Uri $url -TimeoutSec $timeout -ErrorAction Stop
    if ($request.StatusCode -eq 200) {
        Write-Host "Connection successful."
    } else {
        Write-Host "Failed to connect: $($request.StatusCode) - $($request.StatusDescription)"
        [System.Windows.Forms.MessageBox]::Show("Failed to connect to $url.`nStatus Code: $($request.StatusCode)`nStatus Description: $($request.StatusDescription)", "Connection Error", "OK", [System.Windows.Forms.MessageBoxIcon]::Error)
    }
} catch {
    Write-Host "Failed to connect: $_"
    [System.Windows.Forms.MessageBox]::Show("Failed to connect to $url.`nError: $_", "Connection Error", "OK", [System.Windows.Forms.MessageBoxIcon]::Error)
}
