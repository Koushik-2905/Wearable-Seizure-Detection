import 'package:flutter/material.dart';
import 'package:url_launcher/url_launcher.dart';

Future<void> showSeizureAlertDialog(
  BuildContext context,
  Map<String, dynamic> data,
) async {
  final hr = data['hr'];
  final valid = data['valid'] == true;
  final lat = (data['lat'] as num?)?.toDouble();
  final lng = (data['lng'] as num?)?.toDouble();

  await showDialog<void>(
    context: context,
    barrierDismissible: false,
    builder: (ctx) => AlertDialog(
      backgroundColor: const Color(0xFF8B0000),
      title: const Text(
        'SEIZURE DETECTED',
        style: TextStyle(color: Colors.white, fontWeight: FontWeight.bold),
      ),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('Heart Rate: $hr BPM', style: const TextStyle(color: Colors.white)),
          if (valid && lat != null && lng != null)
            TextButton(
              onPressed: () async {
                final url = Uri.parse('https://maps.google.com/?q=$lat,$lng');
                if (await canLaunchUrl(url)) await launchUrl(url, mode: LaunchMode.externalApplication);
              },
              child: const Text('Open in Maps', style: TextStyle(color: Color(0xFF00FF9D))),
            ),
        ],
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(ctx),
          child: const Text('DISMISS', style: TextStyle(color: Colors.white)),
        ),
      ],
    ),
  );
}
