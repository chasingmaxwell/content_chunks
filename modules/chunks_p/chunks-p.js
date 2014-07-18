/**
 * @file
 *   JS behavior for Paragraph chunk type.
 */

(function($) {

  'use strict';

  var autoAddChunks = [];

  // Instantiates the Pen editor.
  Drupal.behaviors.chunksPInPlace = {
    attach: function(context, settings) {

      $('.chunks-field', context).each(function(index, element) {
        var fieldName, langcode, fieldSettings, initializePen;

        fieldName = $(this).attr('field_name');
        langcode = $(this).attr('langcode');
        fieldSettings = settings.chunks.fields[fieldName];

        // Don't do anything if the Pen library isn't loaded.
        if (typeof Pen !== 'function') {
          return;
        }

        // Only initialize Pen if it's needed for a chunk instance.
        for (var instanceName in fieldSettings.instances) {
          if (fieldSettings.instances[instanceName].type === 'p' && fieldSettings.instances[instanceName].settings.edit_in_place) {
            initializePen = true;
            break;
          }
        }

        // Do not continue if there were no paragraph chunk instances with the
        // edit_in_place setting.
        if (!initializePen) {
          return;
        }

        // Sometimes the pen-paragraph-menu doesn't like to go away. Let's force it.
        $('.pen-paragraph-menu').hide();
        $(':input', context).bind('mousedown.hidePenMenu', function() {
          $('.pen-paragraph-menu').hide();
        });

        // Initialize editor for this Paragraph chunk
        $('.p-chunk[contenteditable]').once(function() {
          var editor,
          editorConfig;

          editorConfig = {
            class: 'pen-paragraph',
            editor: this,
            textarea: '<textarea name="content"></textarea>', // fallback for old browsers
            list: ['bold', 'italic', 'underline', 'createlink'],
            stay: false
          };

          editor = new Pen(editorConfig);

          // Hide the regular textarea.
          $(this).parent().find('.form-textarea-wrapper').hide();

          // Update hidden textarea on keyup, blur, paste, and cut events.
          $(this).bind('keyup.chunksPInPlace blur.chunksPInPlace paste.chunksPInPlace cut.chunksPInPlace', function(e) {
            var editableElement = this;

            // Each time the configuration could have changed, copy the data
            // back into the form item so it gets saved when the user submits
            // the form. setTimeout is necessary because some events fire
            // before the markup has changed.
            setTimeout(function() {
              $(editableElement).parent().find('textarea').val($(editableElement).html());
            }, 0);
          });

          // Prevent new lines from being added when user presses "Enter".
          $(this).bind('keydown.chunksPNoDiv', function(e) {
            if (e.keyCode === 13) {
              return false;
            }
          });
        });

        // Trigger blur event on all contenteditable paragraph chunks when the
        // pen toolbar is clicked so we save configuration correctly.
        $('.pen-paragraph-menu').bind('click.chunksPBlur', function() {
          $('.p-chunk[contenteditable]').trigger('blur.chunksPInPlace');
        });
      });
    }
  };

  Drupal.behaviors.chunksPCallbacks = {
    attach: function(context, settings) {

      if (typeof Drupal.settings.chunks.callbacks.restoreConfig.p === 'undefined') {
        // Implements the restoreConfig callback to restore saved configuration.
        Drupal.settings.chunks.callbacks.restoreConfig.p = function(fieldName, langcode, delta) {
          var chunk;

          // Retrieve the Chunk object.
          chunk = Drupal.chunks.fields[fieldName].chunks[delta];

          // Only do anything if this instance of the chunk type is set to be
          // edited in-place.
          if (chunk.chunkInstance.settings.edit_in_place) {
            var classFieldName, pConfig, configuration, configState, inPlaceEditor;

            classFieldName = fieldName.replace(/_/g, '-');
            pConfig = $('#' + fieldName + '-' + delta + '-chunk .p-chunk-type-configuration');

            // Make a copy of the configuration and add the edit_in_place
            // property so we can render a contenteditable paragraph chunk
            // without changing the configuration settings for the chunk.
            configState = chunk.config.get();
            configuration = {};
            for (var prop in configState) {
              configuration[prop] = configState[prop];
            }
            configuration.edit_in_place = true;

            // Generate new markup.
            inPlaceEditor = Drupal.theme.prototype.chunk__p(configuration, fieldName, langcode, delta);

            // Insert that new markup into the editor.
            pConfig.find('.p-chunk').remove();
            pConfig.append(inPlaceEditor);
            Drupal.behaviors.chunksPInPlace.attach(pConfig.parents('.field-type-chunks'), Drupal.settings);
          }
        };
      }

      if (typeof Drupal.settings.chunks.callbacks.initialize.p === 'undefined') {
        // Implements the initialize callback to perform actions on a paragraph
        // Chunk object upon initialization.
        Drupal.settings.chunks.callbacks.initialize.p = function(chunk) {

          // Only perform certain actions if the current chunk is not assigned
          // to a different type.
          if (chunk.field.settings.unlimited && (chunk.chunkType === '' || chunk.chunkType === 'p')) {

            // Add tip so users know about the shortcut.
            $('.p-chunk-type-configuration', chunk.element).append('<div class="description"><strong>Tip:</strong> press <em>Shift + Enter</em> to start writing a new paragraph below this one.</div>');

            // Listen for shift+enter shortcut.
            $('.p-chunk-type-configuration .p-chunk[contenteditable], .p-chunk-type-configuration textarea', chunk.element).bind('keydown.chunksPShortcut', function(e) {
              if (e.keyCode === 13 && e.shiftKey) {

                // Queue the staged chunk to be added automatically as a
                // paragraph chunk.
                autoAddChunks.push({instanceName: chunk.instanceName, prevChunk: chunk});

                chunk.buttons.add.trigger({type: 'mousedown', which: 1});
                $(this).trigger('blur');

                e.preventDefault();
                return false;
              }
            });
          }
        };
      }

      if (typeof Drupal.settings.chunks.callbacks.stagedChunkShown.p === 'undefined') {
        // Implements the stagedChunkShown callback to automatically add the
        // staged chunk as a paragraph chunk if the shortcut set in the
        // initialize callback was invoked.
        Drupal.settings.chunks.callbacks.stagedChunkShown.p = function(chunk, prevChunk) {
          if (prevChunk !== false && chunk.field.settings.unlimited && autoAddChunks.length > 0 && autoAddChunks[0].prevChunk.delta === prevChunk.delta) {
            var addedChunk = autoAddChunks.shift();
            $(':input[name="' + chunk.namePrepend + '[instance]"][value="' + addedChunk.instanceName + '"]', chunk.element).trigger('click.chunkInstanceSelected');
          }
        };
      }
    },
  };

  // Provide a client-side theme implementation for text chunks.
  Drupal.theme.prototype.chunk__p = function(config, fieldName, langcode, delta) {
    var output, contentEditable;

    // Add contenteditable attribute if we're editing this chunk in-place.
    if (typeof config.edit_in_place !== 'undefined') {
      contentEditable = config.edit_in_place ? ' contenteditable' : '';
    }
    else {
      contentEditable = '';
    }

    output = '<p class="chunk p-chunk"' + contentEditable + '>';
    output += config.p;
    output += '</p>';

    return output;
  };

})(jQuery);
