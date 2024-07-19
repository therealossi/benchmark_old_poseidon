import matplotlib.pyplot as plt
import re
from collections import defaultdict
import os

def read_data(file_path):
    data_sets = []
    with open(file_path, 'r') as file:
        for line in file:
            entries = line.strip().split(',')
            operations = defaultdict(float)
            for entry in entries:
                if entry:
                    operation, time_str = entry.split(':')
                    time = float(re.sub(r's$', '', time_str))  # Entferne das 's' und konvertiere zu float
                    operations[operation] += time
            data_sets.append(operations)
    return data_sets

def plot_data(data_sets, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for idx, operations in enumerate(data_sets):
        plt.figure(figsize=(10, 6))
        operations_items = sorted(operations.items())
        labels, values = zip(*operations_items)
        
        plt.bar(labels, values)
        plt.title(f'Operation Times for Dataset {idx + 1}')
        plt.xlabel('Operations')
        plt.ylabel('Time (s)')
        plt.xticks(rotation=45)
        plt.grid(axis='y')

        output_file = os.path.join(output_dir, f'dataset_{idx + 1}.png')
        plt.savefig(output_file)
        plt.close()

if __name__ == '__main__':
    file_path = '/home/ossi/github/test_benchmark/build/benchmark.txt'
    output_dir = 'pic'
    data_sets = read_data(file_path)
    plot_data(data_sets, output_dir)
