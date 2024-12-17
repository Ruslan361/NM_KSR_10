import os
import pandas as pd
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import numpy as np
import openpyxl
from openpyxl.drawing.image import Image
import argparse

def create_output_directory(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)

def plot_heat_equation_results(csv_file, save_3d_plot=False, output_dir="plots", show_graph=True):
    create_output_directory(output_dir)
    
    # Load data from CSV
    data = pd.read_csv(csv_file)

    # Extract bounds from data
    x_max = max(data['x'])
    t_max = max(data['t'])
    v_max = max(data['v'])
    x_min = min(data['x'])
    t_min = min(data['t'])
    v_min = min(data['v'])

    # Create a 3D plot
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Plot the data
    ax.plot_trisurf(data['x'], data['t'], data['v'], cmap='viridis')
    intervals = 20
    # # Add boundary condition u(x, 0) = 1 - x^2
    # x_curve = np.linspace(x_min, x_max, intervals)
    # t_curve = np.zeros_like(x_curve)
    # v_curve = 1 - x_curve**2
    # linewidth = 1
    # ax.plot(x_curve, t_curve, v_curve, label='u(x, 0) = 1 - x^2', color='red', linewidth=linewidth)

    # # Add boundary condition u(0, t) = cos(t)
    # t_curve = np.linspace(t_min, t_max, intervals)
    # x_curve = np.zeros_like(t_curve)
    # v_curve = np.cos(t_curve)
    # ax.plot(x_curve, t_curve, v_curve, label='u(0, t) = cos(t)', color='blue', linewidth=linewidth)

    # # Add boundary condition u(1, t) = sin(4t)
    # t_curve = np.linspace(t_min, t_max, intervals)
    # x_curve = np.ones_like(t_curve) * x_max
    # v_curve = np.sin(4 * t_curve)
    # ax.plot(x_curve, t_curve, v_curve, label='u(1, t) = sin(4t)', color='green', linewidth=linewidth)

    # Configure plot
    ax.set_xlabel('x(Координата)')
    ax.set_ylabel('t(Время)')
    ax.set_zlabel('Температура')
    ax.set_title('Результаты численного решения уравнения теплопроводности')
    #ax.legend()

    # Function to rotate the view clockwise
    def rotate_clockwise(event):
        ax.view_init(elev=30, azim=ax.azim + 15)
        fig.canvas.draw_idle()  # Redraw the figure

    # Function to rotate the view counterclockwise
    def rotate_counterclockwise(event):
        ax.view_init(elev=30, azim=ax.azim - 15)
        fig.canvas.draw_idle()  # Redraw the figure

    # Add buttons for rotation
    ax_button_cw = plt.axes([0, 0.05, 0.5, 0.075])  # Position of the clockwise button [left, bottom, width, height]
    button_cw = Button(ax_button_cw, 'Вращать влево')
    button_cw.on_clicked(rotate_clockwise)

    ax_button_ccw = plt.axes([0.5, 0.05, 0.5, 0.075])  # Position of the counterclockwise button [left, bottom, width, height]
    button_ccw = Button(ax_button_ccw, 'Вращать вправо')
    button_ccw.on_clicked(rotate_counterclockwise)
    #fig = plt.figure()
    #fig.canvas.set_window_title('Результаты численного решения уравнения теплопроводности')
    if save_3d_plot:
        plt.savefig(os.path.join(output_dir, "3d_plot.png"))
    if show_graph:
        plt.show()
    # plt.show()

def plot_individual_layer(df, x_idx, filename):
    layer = df[df['x'] == x_idx].sort_values(by='t')
    plt.figure(figsize=(10, 6))
    plt.plot(layer['t'], layer['v'])
    plt.xlabel("Время")
    plt.ylabel("Температура")
    plt.title(f"Слой x={x_idx}")
    plt.grid()
    plt.savefig(filename)
    plt.close()

def plot_first_and_last_layers(df, filename):
    plt.figure(figsize=(12, 8))
    unique_x = df['x'].unique()
    selected_indices = list(unique_x[:5]) + list(unique_x[-5:])

    for x_value in selected_indices:
        layer = df[df['x'] == x_value].sort_values(by='t')
        plt.plot(layer['t'], layer['v'], label=f"Слой x={x_value}")

    plt.xlabel("Время")
    plt.ylabel("Температура")
    plt.title("Первые и последние 5 слоёв")
    plt.legend()
    plt.grid()
    plt.savefig(filename)
    plt.close()

def plot_individual_time_slice(df, t_idx, filename):
    slice = df[df['t'] == t_idx].sort_values(by='x')
    plt.figure(figsize=(10, 6))
    plt.plot(slice['x'], slice['v'])
    plt.xlabel("Координата (x)")
    plt.ylabel("Температура")
    plt.title(f"Слой t={t_idx}")
    plt.grid()
    plt.savefig(filename)
    plt.close()

def save_results_and_plots_to_excel(filename, output_dir="plots"):
    create_output_directory(output_dir)
    
    df = pd.read_csv(filename)
    excel_file = "heat_equation_results_with_plots.xlsx"

    # Pivot the dataframe to have x as columns, t as rows, and v as values
    pivot_df = df.pivot(index='t', columns='x', values='v')

    with pd.ExcelWriter(excel_file, engine="openpyxl") as writer:
        pivot_df.to_excel(writer, sheet_name="Overview", float_format="%.6f")
        pd.DataFrame().to_excel(writer, sheet_name="Графики")
        pd.DataFrame().to_excel(writer, sheet_name="Срезы по времени")

    wb = openpyxl.load_workbook(excel_file)

    sheet = wb["Overview"]
    plot_image = os.path.join(output_dir, "overview_plot.png")
    plot_first_and_last_layers(df, plot_image)
    img = Image(plot_image)
    img.anchor = "A20"
    sheet.add_image(img)

    sheet = wb["Графики"]
    row_offset = 10

    unique_x = df['x'].unique()
    selected_x_values = np.concatenate([unique_x[:5], unique_x[-5:]])

    for i, x_value in enumerate(selected_x_values, start=1):
        plot_image = os.path.join(output_dir, f"layer_{i}_plot.png")
        plot_individual_layer(df, x_value, plot_image)
        img = Image(plot_image)
        img.anchor = f"A{row_offset}"
        sheet.add_image(img)
        row_offset += 20

    sheet = wb["Срезы по времени"]
    row_offset = 1

    unique_t = df['t'].unique()
    selected_t_values = np.concatenate([unique_t[:5], unique_t[-5:]])

    for i, t_value in enumerate(selected_t_values, start=1):
        plot_image = os.path.join(output_dir, f"time_slice_{i}_plot.png")
        plot_individual_time_slice(df, t_value, plot_image)
        img = Image(plot_image)
        img.anchor = f"A{row_offset}"
        sheet.add_image(img)
        row_offset += 20

    wb.save(excel_file)
    print(f"Results and plots saved to '{excel_file}'")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot heat equation results")
    parser.add_argument("file", type=str, help="results.csv file")
    parser.add_argument("save_excel", type=str, help="save excel")
    parser.add_argument("show_graph", type=str, help="show graph")
    parser.add_argument("--save_3d_plot", action="store_true", help="save 3D plot")
    parser.add_argument("--output_dir", type=str, default="plots", help="directory to save plots")
    args = parser.parse_args()
    #print(args)
    if args.show_graph == "True":
        plot_heat_equation_results(args.file, args.save_3d_plot, args.output_dir)
    else:
        plot_heat_equation_results(args.file, True, args.output_dir, False)
    if args.save_excel == "True": 
        save_results_and_plots_to_excel(args.file, args.output_dir)