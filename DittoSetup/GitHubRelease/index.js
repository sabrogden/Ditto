const axios = require('axios');
const fs = require('fs/promises');
const { env } = require('process');

async function GetNightlyRelease() {

    const config = {
        method: 'get',
        headers: { 'X-GitHub-Api-Version': '2022-11-28',  'Authorization': `token ${env.token}`},
        url: 'https://api.github.com/repos/sabrogden/Ditto/releases'
    }

    let res = await axios(config)

    for (const r of res.data) {

        if(r.tag_name == 'nightly')
        {
            return r;
        }
    }

    return undefined;
}

async function DeleteOldAssets(nightlyRelease) {

    let oldAssetIds = [];
    for(const asset of nightlyRelease.assets) {        
        const config = {
            method: 'DELETE',
            headers: { 'X-GitHub-Api-Version': '2022-11-28',  'Authorization': `token ${env.token}`},
            url: `https://api.github.com/repos/sabrogden/Ditto/releases/assets/${asset.id}`                
        }
        
        console.log(`Deleting file ${asset.name}, id: ${asset.id}`);

        let res = await axios(config);
    }
}

async function UploadFiles(nightlyRelease) {
    let files = await fs.readdir(env.uploadPath);
    for(const file of files) {

        if(file.startsWith('Ditto')) {

            let fullPath = `${env.uploadPath}${file}`;

            let fileBytes = await fs.readFile(fullPath);

            const config = {
                method: 'POST',
                headers: { 
                            'X-GitHub-Api-Version': '2022-11-28',  
                            'Authorization': `token ${env.token}`, 
                            'Content-Type': 'application/octet-stream',
                            'Content-Length': fileBytes.length
                        },
                url: `https://uploads.github.com/repos/sabrogden/Ditto/releases/${nightlyRelease.id}/assets?name=${file}`,              
                data: fileBytes
            }

            console.log(`Uploading file ${fullPath}`);
        
            let res = await axios(config);
        }
    }
}

async function UpdateReleaseNotes(nightlyRelease) {
    
    let releaseNotes = '';

    {
        const config = {
            method: 'POST',
            headers: { 
                        'X-GitHub-Api-Version': '2022-11-28',  
                        'Authorization': `token ${env.token}`
                        },
            url: `https://api.github.com/repos/sabrogden/Ditto/releases/generate-notes`,
            data: {                        
                    "tag_name": env.tag,
                    "previous_tag_name": env.previous_tag
                }
        }
        

        console.log(`Generating release notes`);
    
        let res = await axios(config);

        releaseNotes = res.data.body;
    }


    {

        const config = {
            method: 'PATCH',
            headers: { 
                        'X-GitHub-Api-Version': '2022-11-28',  
                        'Authorization': `token ${env.token}`
                    },
            url: `https://api.github.com/repos/sabrogden/Ditto/releases/${nightlyRelease.id}`,
            data: {
                "body": releaseNotes
            }
        }

        console.log(`Uplaoding release notes: ${releaseNotes}`);
    
        let res = await axios(config);
    }        
}


async function Run() {
	
	console.log(`running publish build: ${env.token}`);
	
    let nightlyRelease = await GetNightlyRelease();

    await DeleteOldAssets(nightlyRelease);

    await UploadFiles(nightlyRelease);

    await UpdateReleaseNotes(nightlyRelease);
}

Run();