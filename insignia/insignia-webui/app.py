from flask import Flask, render_template, request, redirect, url_for, send_from_directory
import os
import uuid

UPLOAD_FOLDER = 'uploads'
ALLOWED_EXTENSIONS = {'fasta', 'fa', 'fna', 'txt'}

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Helpers
def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/upload', methods=['POST'])
def upload_files():
    target_file = request.files.get('target')
    exclude_file = request.files.get('exclude')
    
    if not target_file or not allowed_file(target_file.filename):
        return "Invalid target file", 400
    if not exclude_file or not allowed_file(exclude_file.filename):
        return "Invalid exclude file", 400

    session_id = str(uuid.uuid4())
    session_dir = os.path.join(app.config['UPLOAD_FOLDER'], session_id)
    os.makedirs(session_dir)

    target_path = os.path.join(session_dir, 'target.fasta')
    exclude_path = os.path.join(session_dir, 'exclude.fasta')
    target_file.save(target_path)
    exclude_file.save(exclude_path)

    # Redirect to processing page (we'll implement this next)
    return redirect(url_for('process', session_id=session_id))

@app.route('/process/<session_id>')
def process(session_id):
    # TODO: Run kmer/intersect/unique-mer pipeline here
    return f"Processing session: {session_id} — [signature logic pending]"

if __name__ == '__main__':
    os.makedirs(UPLOAD_FOLDER, exist_ok=True)
    app.run(debug=True)

