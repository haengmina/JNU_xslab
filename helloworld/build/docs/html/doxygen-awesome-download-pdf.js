/**

Doxygen Awesome
https://github.com/jothepro/doxygen-awesome-css

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

class DoxygenAwesomeDownloadPDF extends HTMLElement {
    // SVG icons from https://fonts.google.com/icons
    // Licensed under the Apache 2.0 license:
    // https://www.apache.org/licenses/LICENSE-2.0.html
    static darkModeIcon = `<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 -960 960 960" width="24" fill="#FCBF00"><path d="M480-320 280-520l56-58 104 104v-326h80v326l104-104 56 58-200 200ZM240-160q-33 0-56.5-23.5T160-240v-120h80v120h480v-120h80v120q0 33-23.5 56.5T720-160H240Z"/></svg>`
    static lightModeIcon = `<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 -960 960 960" width="24" fill="#FE9700"><path d="M480-320 280-520l56-58 104 104v-326h80v326l104-104 56 58-200 200ZM240-160q-33 0-56.5-23.5T160-240v-120h80v120h480v-120h80v120q0 33-23.5 56.5T720-160H240Z"/></svg>`
    static title = "PDF Download"

    static init() {
        $(function() {
            $(document).ready(function() {
                const downloadButton = document.createElement('doxygen-awesome-download-pdf')
                downloadButton.title = DoxygenAwesomeDownloadPDF.title
                downloadButton.updateIcon()

                window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', event => {
                    downloadButton.updateIcon()
                })
                document.addEventListener("visibilitychange", visibilityState => {
                    if (document.visibilityState === 'visible') {
                        downloadButton.updateIcon()
                    }
                });

                $(document).ready(function(){
                    document.getElementById("MSearchBox").parentNode.appendChild(downloadButton)
                })
                $(window).resize(function(){
                    document.getElementById("MSearchBox").parentNode.appendChild(downloadButton)
                })
            })
        })
    }

    constructor() {
        super();
        this.onclick=this.download_PDF
    }

    updateIcon() {
        if(DoxygenAwesomeDarkModeToggle.darkModeEnabled) {
            this.innerHTML = DoxygenAwesomeDownloadPDF.darkModeIcon
        } else {
            this.innerHTML = DoxygenAwesomeDownloadPDF.lightModeIcon
        }
    }

    download_PDF() {
        let file_name = document.getElementById('projectname').innerText.replace(/[ ]+\xA0/, ' - ') + '.pdf';
        let element = document.createElement('a');
        document.body.appendChild(element);
        element.setAttribute('target','_blank');
        element.setAttribute('href', 'print.pdf');
        element.setAttribute('download', file_name);
        element.click();
        document.body.removeChild(element);
    }
}

customElements.define("doxygen-awesome-download-pdf", DoxygenAwesomeDownloadPDF);
