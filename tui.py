#!/usr/bin/env python3

from textual.app import App, ComposeResult
from textual.containers import Container, Horizontal, Vertical
from textual.widgets import Header, Footer, Button, Static, Input, Checkbox, Log, Label, DirectoryTree
from textual.screen import ModalScreen
from textual.binding import Binding
from textual import on
import subprocess
import asyncio
import os


class FilePickerScreen(ModalScreen):
	"""Dosya/Klasör seçici ekran"""

	BINDINGS = [
		Binding("x", "btn_select", "Seç", show=False),
		Binding("escape,z", "btn_cancel", "İptal", show=False),
	]

	CSS = """
	FilePickerScreen {
		align: center middle;
	}

	#picker-container {
		width: 70;
		height: 25;
		background: #2b3339;
		border: thick #a7c080;
	}

	#picker-title {
		height: 3;
		content-align: center middle;
		background: #a7c080;
		color: #2b3339;
		text-style: bold;
	}

	#picker-path {
		height: 3;
		padding: 0 2;
		background: #232a2e;
		color: #d3c6aa;
		content-align: left middle;
	}

	DirectoryTree {
		height: 1fr;
		background: #232a2e;
		scrollbar-gutter: stable;
	}

	#picker-buttons {
		height: 4;
		align: center middle;
		background: #2b3339;
	}

	#picker-buttons Button {
		margin: 0 1;
		min-width: 15;
	}
	"""

	def __init__(self, title: str = "Dosya/Klasör Seç", start_path: str = "."):
		super().__init__()
		self.picker_title = title
		self.start_path = start_path
		self.selected_path = None

	def compose(self) -> ComposeResult:
		with Container(id="picker-container"):
			yield Static(self.picker_title, id="picker-title")
			yield Static(f"📂 {os.path.abspath(self.start_path)}", id="picker-path")
			yield DirectoryTree(self.start_path, id="file_tree")
			with Horizontal(id="picker-buttons"):
				yield Button("✓ Seç [x]", id="btn_select", variant="success")
				yield Button("✗ İptal [z]", id="btn_cancel", variant="error")

	@on(DirectoryTree.FileSelected)
	def on_file_selected(self, event: DirectoryTree.FileSelected) -> None:
		"""Dosya seçildiğinde"""
		self.selected_path = str(event.path)
		path_display = self.query_one("#picker-path", Static)
		path_display.update(f"📄 {self.selected_path}")

	@on(DirectoryTree.DirectorySelected)
	def on_directory_selected(self, event: DirectoryTree.DirectorySelected) -> None:
		"""Klasör seçildiğinde"""
		self.selected_path = str(event.path)
		path_display = self.query_one("#picker-path", Static)
		path_display.update(f"📂 {self.selected_path}")

	@on(Button.Pressed, "#btn_select")
	def on_select(self) -> None:
		"""Seçimi onayla"""
		if self.selected_path:
			self.dismiss(self.selected_path)
		else:
			self.dismiss(os.path.abspath(self.start_path))

	@on(Button.Pressed, "#btn_cancel")
	def on_cancel(self) -> None:
		"""Seçimi iptal et"""
		self.dismiss(None)

	def action_btn_select(self) -> None:
		"""Seç - klavye kısayolu [x]"""
		self.on_select()

	def action_btn_cancel(self) -> None:
		"""İptal - klavye kısayolu [z/escape]"""
		self.on_cancel()


class CpppTUI(App):

	CSS = """
	Screen {
		align: center middle;
		background: #1e2326;
	}

	#app-container {
		width: 85;
		height: 38;
		background: #2b3339;
		border: thick #a7c080;
	}

	#title-bar {
		height: 3;
		content-align: center middle;
		background: #a7c080;
		color: #2b3339;
		text-style: bold;
	}

	.section {
		background: #2b3339;
		padding: 1 2;
		border-bottom: solid #374247;
	}

	.input-row {
		height: 3;
		margin: 0 0 0 0;
		align: left middle;
	}

	.input-label {
		width: 18;
		content-align: left middle;
		color: #d3c6aa;
		text-style: bold;
	}

	Input {
		width: 1fr;
		height: 3;
		background: #232a2e;
		color: #d3c6aa;
		border: solid #4f585e;
	}

	Input:focus {
		border: solid #a7c080;
	}

	.browse-btn {
		width: 8;
		min-width: 8;
		margin-left: 1;
		background: #7fbbb3;
		color: #2b3339;
	}

	.browse-btn:hover {
		background: #83c092;
	}

	.options-row {
		height: auto;
		align: left middle;
		margin: 0 0;
	}

	Checkbox {
		background: transparent;
		color: #d3c6aa;
		margin: 0 1 0 0;
		height: 3;
	}

	#logs-section {
		height: 10;
		padding: 0;
		margin: 0 2;
	}

	#logs-title {
		height: 1;
		background: #dbbc7f;
		color: #2b3339;
		content-align: center middle;
		text-style: bold;
	}

	Log {
		height: 9;
		background: #232a2e;
		color: #d3c6aa;
		border: none;
		padding: 0 1;
	}

	#buttons-section {
		height: 4;
		padding: 1 2;
		align: center middle;
	}

	Button {
		min-width: 12;
		margin: 0 1;
		height: 3;
	}

	.btn-help {
		background: #7fbbb3;
		color: #2b3339;
	}

	.btn-help:hover {
		background: #83c092;
	}

	.btn-start {
		background: #a7c080;
		color: #2b3339;
	}

	.btn-start:hover {
		background: #b4d292;
	}

	.btn-stop {
		background: #e67e80;
		color: #2b3339;
	}

	.btn-stop:hover {
		background: #ef8a8c;
	}

	Footer {
		background: #374247;
		color: #d3c6aa;
	}

	Header {
		background: #a7c080;
		color: #2b3339;
	}
	"""

	BINDINGS = [
		Binding("q", "quit", "Çıkış", show=True),
		Binding("ctrl+c", "quit", "Çıkış", show=False),
		Binding("s", "toggle_start", "Başlat", show=True),
		Binding("h", "show_help", "Yardım", show=True),
	]

	def __init__(self):
		super().__init__()
		self.process = None
		self.process_running = False

	def compose(self) -> ComposeResult:
		"""Arayüz bileşenlerini oluştur"""
		yield Header()
		
		with Container(id="app-container"):
			yield Static("╔═══ cppp - tui ═══╗", id="title-bar")
			
			# Dosya yolları
			with Vertical(classes="section"):
				with Horizontal(classes="input-row"):
					yield Label("Kaynak (-i):", classes="input-label")
					yield Input(placeholder="Örn: ./dosya.txt", id="input_path")
					yield Button("📁", id="btn_browse_input", classes="browse-btn")
				
				with Horizontal(classes="input-row"):
					yield Label("Hedef (-o):", classes="input-label")
					yield Input(placeholder="Örn: /hedef/klasor/", id="output_path")
					yield Button("📁", id="btn_browse_output", classes="browse-btn")
				
				with Horizontal(classes="input-row"):
					yield Label("Parça Sayısı (-p):", classes="input-label")
					yield Input(value="4", placeholder="1-50", id="parts")
			
			# Seçenekler
			with Horizontal(classes="section options-row"):
				yield Checkbox("Detaylı (-v)", id="verbose", value=True)
				yield Checkbox("Üzerine Yaz (-f)", id="force")
				yield Checkbox("SHA-256 (-c)", id="checksum")
			
			# Loglar
			with Vertical(id="logs-section"):
				yield Static("─── İŞLEM KAYITLARI ───", id="logs-title")
				yield Log(id="logs", auto_scroll=True)
			
			# Butonlar
			with Horizontal(id="buttons-section"):
				yield Button("📖 Yardım", id="btn_help", classes="btn-help")
				yield Button("▶ Başlat", id="btn_start", classes="btn-start")
		
		yield Footer()

	async def on_mount(self) -> None:
		"""Uygulama başlatıldığında"""
		log = self.query_one("#logs", Log)
		log.write_line("╔═════════════════╗")
		log.write_line("║   cppp - tui    ║")
		log.write_line("╚═════════════════╝")
		log.write_line("")
		log.write_line("Hızlı Başlangıç:")
		log.write_line("  1. Kaynak: Kopyalanacak dosya/klasör")
		log.write_line("  2. Hedef: Dosyanın gideceği yer")
		log.write_line("     • Klasör ise: /hedef/klasor/")
		log.write_line("     • Dosya ise: /hedef/yeni_dosya.txt")
		log.write_line("  3. Parça sayısını ayarlayın")
		log.write_line("  4. [s] tuşu ile başlatın")
		log.write_line("")
		log.write_line("[s] Başlat | [h] Yardım | [q] Çıkış")
		log.write_line("")

	@on(Button.Pressed, "#btn_browse_input")
	def browse_input(self) -> None:
		"""Kaynak dosya seç"""
		current_path = self.query_one("#input_path", Input).value or "."
		
		def handle_result(result):
			if result:
				self.query_one("#input_path", Input).value = result
		
		self.push_screen(
			FilePickerScreen("Kaynak Dosya/Klasör Seç", current_path),
			handle_result
		)

	@on(Button.Pressed, "#btn_browse_output")
	def browse_output(self) -> None:
		"""Hedef konum seç"""
		current_path = self.query_one("#output_path", Input).value or "."
		
		def handle_result(result):
			if result:
				self.query_one("#output_path", Input).value = result
		
		self.push_screen(
			FilePickerScreen("Hedef Konum Seç", current_path),
			handle_result
		)

	@on(Button.Pressed, "#btn_help")
	def on_help(self) -> None:
		"""Yardım göster"""
		self.action_show_help()

	def action_show_help(self) -> None:
		"""Yardım bilgisini göster"""
		log = self.query_one("#logs", Log)
		log.clear()
		log.write_line("╔════════════════════════════════════════════════╗")
		log.write_line("║          cppp - Yardım Kılavuzu                ║")
		log.write_line("╚════════════════════════════════════════════════╝")
		log.write_line("")
		log.write_line("📋 cppp Nedir?")
		log.write_line("  Paralel dosya kopyalama aracı. Dosyaları")
		log.write_line("  parçalara bölerek aynı anda kopyalar.")
		log.write_line("")
		log.write_line("📂 Hedef Yol Nasıl Belirtilir?")
		log.write_line("  • Klasöre kopyala: /hedef/klasor/")
		log.write_line("  • Yeni isimle kaydet: /hedef/yeni_isim.txt")
		log.write_line("  • Mevcut klasöre: ./yerel_klasor/")
		log.write_line("")
		log.write_line("📝 Örnekler:")
		log.write_line("  Kaynak: video.mp4  → Hedef: /yedek/")
		log.write_line("  Sonuç: /yedek/video.mp4")
		log.write_line("")
		log.write_line("  Kaynak: dosya.txt  → Hedef: /tmp/yeni.txt")
		log.write_line("  Sonuç: /tmp/yeni.txt")
		log.write_line("")
		log.write_line("⚙️  Seçenekler:")
		log.write_line("  • Detaylı (-v): İlerleme ve hız gösterir")
		log.write_line("  • Üzerine Yaz (-f): Var olan dosyaları değiştirir")
		log.write_line("  • SHA-256 (-c): Bütünlük kontrolü yapar")
		log.write_line("")
		log.write_line("💡 Parça Sayısı:")
		log.write_line("  • Küçük (<100MB): 1-4 parça")
		log.write_line("  • Orta (100MB-1GB): 4-10 parça")
		log.write_line("  • Büyük (>1GB): 10-20 parça")
		log.write_line("")
		log.write_line("⌨️  Dosya Seçici Tuşları:")
		log.write_line("  [x] → Seçili dosyayı onayla")
		log.write_line("  [z] veya [Esc] → İptal")
		log.write_line("")
		log.write_line("════════════════════════════════════════════════")

	@on(Button.Pressed, "#btn_start")
	async def on_start_stop(self) -> None:
		"""İşlemi başlat veya durdur"""
		if self.process_running:
			await self.stop_process()
		else:
			await self.start_process()

	async def start_process(self) -> None:
		"""cppp işlemini başlat"""
		log = self.query_one("#logs", Log)
		button = self.query_one("#btn_start", Button)
		
		# Değerleri al
		input_path = self.query_one("#input_path", Input).value.strip()
		output_path = self.query_one("#output_path", Input).value.strip()
		parts = self.query_one("#parts", Input).value.strip()
		
		# Seçenekleri al
		verbose = self.query_one("#verbose", Checkbox).value
		force = self.query_one("#force", Checkbox).value
		checksum = self.query_one("#checksum", Checkbox).value
		
		# Kontroller
		if not input_path:
			log.write_line("")
			log.write_line("❌ Kaynak yolu boş!")
			return
		
		if not output_path:
			log.write_line("")
			log.write_line("❌ Hedef yolu boş!")
			return
		
		# Parça sayısını kontrol et
		try:
			parts_int = int(parts) if parts else 1
			if parts_int < 1 or parts_int > 50:
				log.write_line("")
				log.write_line("❌ Parça sayısı 1-50 arası olmalı!")
				return
		except ValueError:
			log.write_line("")
			log.write_line("❌ Geçersiz parça sayısı!")
			return
		
		# cppp binary'sini bul
		cppp_paths = ["./build/cppp", "./cppp", "cppp"]
		cppp_bin = None
		for path in cppp_paths:
			if os.path.exists(path) or path == "cppp":
				cppp_bin = path
				break
		
		if not cppp_bin:
			log.write_line("")
			log.write_line("❌ cppp bulunamadı!")
			log.write_line("Kurulum: cd build && cmake .. && make")
			return
		
		# Komutu oluştur
		cmd = [cppp_bin, "-i", input_path, "-o", output_path]
		if parts:
			cmd.extend(["-p", parts])
		if verbose:
			cmd.append("-v")
		if force:
			cmd.append("-f")
		if checksum:
			cmd.append("-c")
		
		log.clear()
		log.write_line("════════════════════════════════════════════════")
		log.write_line("🚀 Kopyalama Başlatıldı")
		log.write_line("════════════════════════════════════════════════")
		log.write_line("")
		log.write_line("📌 " + " ".join(cmd))
		log.write_line("")
		log.write_line("────────────────────────────────────────────────")
		
		try:
			# UI'yi güncelle
			self.process_running = True
			button.label = "⏹ Durdur"
			button.remove_class("btn-start")
			button.add_class("btn-stop")
			
			# İşlemi çalıştır
			process = await asyncio.create_subprocess_exec(
				*cmd,
				stdout=asyncio.subprocess.PIPE,
				stderr=asyncio.subprocess.PIPE
			)
			
			self.process = process
			
			# ANSI escape kodlarını temizle
			import re
			ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
			
			# Çıktıyı oku
			async def read_stream(stream, prefix=""):
				buffer = ""
				progress_count = 0
				while True:
					chunk = await stream.read(512)
					if not chunk:
						break
					
					decoded = chunk.decode('utf-8', errors='ignore')
					
					# \r varsa progress bar güncellemesi (her 5 güncellemede bir göster)
					if '\r' in decoded:
						parts = decoded.split('\r')
						for part in parts:
							if part.strip():
								clean = ansi_escape.sub('', part).strip()
								if clean and not clean.startswith('[?25'):
									# Progress bar mı kontrol et
									if '[' in clean and '%' in clean and 'MB/s' in clean:
										progress_count += 1
										# Her 5 güncellemede bir göster
										if progress_count % 5 == 0:
											log.write_line(prefix + clean)
											await asyncio.sleep(0.02)
									else:
										log.write_line(prefix + clean)
										await asyncio.sleep(0.01)
					else:
						buffer += decoded
						lines = buffer.split('\n')
						buffer = lines[-1]
						
						for line in lines[:-1]:
							clean = ansi_escape.sub('', line).strip()
							if clean and not clean.startswith('[?25'):
								log.write_line(prefix + clean)
								await asyncio.sleep(0.01)
			
			await asyncio.gather(
				read_stream(process.stdout, ""),
				read_stream(process.stderr, "⚠️  ")
			)
			
			await process.wait()
			
			log.write_line("")
			log.write_line("────────────────────────────────────────────────")
			if process.returncode == 0:
				log.write_line("✅ Başarıyla Tamamlandı!")
			else:
				log.write_line(f"❌ Başarısız! (Kod: {process.returncode})")
			log.write_line("════════════════════════════════════════════════")
			
		except FileNotFoundError:
			log.write_line("")
			log.write_line("❌ cppp bulunamadı!")
			log.write_line("Kurulum:")
			log.write_line("  mkdir build && cd build")
			log.write_line("  cmake .. && make")
		except Exception as e:
			log.write_line("")
			log.write_line(f"❌ Hata: {str(e)}")
		finally:
			self.process_running = False
			button.label = "▶ Başlat"
			button.remove_class("btn-stop")
			button.add_class("btn-start")
			self.process = None

	async def stop_process(self) -> None:
		"""Çalışan işlemi durdur"""
		log = self.query_one("#logs", Log)
		button = self.query_one("#btn_start", Button)
		
		if self.process:
			log.write_line("")
			log.write_line("⏹️  Durduruluyor...")
			self.process.terminate()
			try:
				await asyncio.wait_for(self.process.wait(), timeout=5.0)
				log.write_line("✅ Durduruldu.")
			except asyncio.TimeoutError:
				self.process.kill()
				await self.process.wait()
				log.write_line("✅ Zorla sonlandırıldı.")
		
		self.process_running = False
		button.label = "▶ Başlat"
		button.remove_class("btn-stop")
		button.add_class("btn-start")

	def action_toggle_start(self) -> None:
		"""Başlat/Durdur - klavye kısayolu"""
		button = self.query_one("#btn_start", Button)
		button.press()


def main():
	"""Uygulamayı çalıştır"""
	app = CpppTUI()
	app.run()


if __name__ == "__main__":
	main()