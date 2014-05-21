/**
 * @file Provide a client-side theme implementation for text chunks.
 */

(function($) {
  Drupal.theme.prototype.chunk__p = function(config) {
    var output = '<p class="chunk p-chunk">';
    output += Drupal.checkPlain(config.p);
    output += '</p>';
    return output;
  };
  Drupal.theme.prototype.chunk_callback__p = function(config, classPrepend) {
    // Initialize editor for this Paragraph chunk
    $(classPrepend + 'preview .p-chunk').each(function() {
      var editor,
          editorConfig;

      editorConfig = {
        class: 'pen-editor',
        debug: true,
        editor: this,
        textarea: '<textarea name="content"></textarea>', // fallback for old browsers
        list: ['bold', 'italic', 'underline', 'createlink']
      };
      editor = new Pen(editorConfig);
    }).live('keyup', function() {
      // Each keystroke, copy the data back into the form item so it gets saved
      // when the user submits the form.
      console.log('#edit-' + classPrepend.substring(1, classPrepend.length - 2) + 'und-' + classPrepend.substring(classPrepend.length - 2) + 'configuration-p-p');
      $('#edit-' + classPrepend.substring(1, classPrepend.length - 2) + 'und-' + classPrepend.substring(classPrepend.length - 2) + 'configuration-p-p').val($(this).html());
    });

  };
})(jQuery);