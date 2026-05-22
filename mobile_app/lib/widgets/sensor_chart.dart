import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';

class SensorChart extends StatelessWidget {
  const SensorChart({super.key, required this.confidenceHistory});

  final List<double> confidenceHistory;

  @override
  Widget build(BuildContext context) {
    if (confidenceHistory.isEmpty) {
      return const SizedBox(
        height: 120,
        child: Center(child: Text('Waiting for sensor stream…', style: TextStyle(color: Colors.white54))),
      );
    }

    final spots = confidenceHistory
        .asMap()
        .entries
        .map((e) => FlSpot(e.key.toDouble(), e.value))
        .toList();

    return SizedBox(
      height: 140,
      child: LineChart(
        LineChartData(
          gridData: const FlGridData(show: false),
          titlesData: const FlTitlesData(show: false),
          borderData: FlBorderData(show: false),
          minY: 0,
          maxY: 1,
          lineBarsData: [
            LineChartBarData(
              spots: spots,
              isCurved: true,
              color: const Color(0xFF00FF9D),
              barWidth: 2,
              dotData: const FlDotData(show: false),
            ),
          ],
        ),
      ),
    );
  }
}
