import 'package:geolocator/geolocator.dart';
import 'package:permission_handler/permission_handler.dart';

class PermissionsService {
  static Future<bool> ensureBleAndLocation() async {
    final btScan = await Permission.bluetoothScan.request();
    final btConnect = await Permission.bluetoothConnect.request();
    final loc = await Permission.locationWhenInUse.request();

    if (!btScan.isGranted || !btConnect.isGranted) return false;
    if (!loc.isGranted) return false;

    final serviceEnabled = await Geolocator.isLocationServiceEnabled();
    if (!serviceEnabled) return false;

    LocationPermission perm = await Geolocator.checkPermission();
    if (perm == LocationPermission.denied) {
      perm = await Geolocator.requestPermission();
    }
    return perm == LocationPermission.always ||
        perm == LocationPermission.whileInUse;
  }
}
