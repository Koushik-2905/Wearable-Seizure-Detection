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
        child: Center(
          child: Text('Waiting for live data…', style: TextStyle(color: Colors.white38)),
        ),
      );
    }

    final spots = confidenceHistory
        .asMap()
        .entries
        .map((e) => FlSpot(e.key.toDouble(), e.value * 100))
        .toList();

    return SizedBox(
      height: 120,
      child: LineChart(
        LineChartData(
          gridData: const FlGridData(show: false),
          titlesData: const FlTitlesData(show: false),
          borderData: FlBorderData(show: false),
          minY: 0,
          maxY: 100,
          lineBarsData: [
            LineChartBarData(
              spots: spots,
              isCurved: false,
              color: const Color(0xFF00FF9D),
              barWidth: 2,
              dotData: const FlDotData(show: false),
              belowBarData: BarAreaData(
                show: true,
                color: const Color(0xFF00FF9D).withOpacity(0.12),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
