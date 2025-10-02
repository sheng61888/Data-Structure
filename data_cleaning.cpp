#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <map>
#include <set>
using namespace std;

struct JobData
{
    int id;
    string description;
    string category;

    JobData(int i = 0, const string &desc = "", const string &cat = "Other") : id(i), description(desc), category(cat) {}
};

string toLowerCase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

string classifyJob(const string& description) {
    string lowerDesc = toLowerCase(description);
    
    // Data roles
    if (lowerDesc.find("data analyst") != string::npos || 
        lowerDesc.find("data scientist") != string::npos ||
        lowerDesc.find("business analyst") != string::npos ||
        lowerDesc.find("analytics") != string::npos) {
        return "Data Analyst";
    }
    
    // ML/AI roles
    if (lowerDesc.find("machine learning") != string::npos ||
        lowerDesc.find("ml engineer") != string::npos ||
        lowerDesc.find("ai engineer") != string::npos ||
        lowerDesc.find("artificial intelligence") != string::npos) {
        return "ML Engineer";
    }
    
    // Software development roles
    if (lowerDesc.find("software engineer") != string::npos ||
        lowerDesc.find("software developer") != string::npos ||
        lowerDesc.find("full stack") != string::npos ||
        lowerDesc.find("backend") != string::npos ||
        lowerDesc.find("frontend") != string::npos) {
        return "Software Engineer";
    }
    
    // DevOps roles
    if (lowerDesc.find("devops") != string::npos ||
        lowerDesc.find("cloud engineer") != string::npos ||
        lowerDesc.find("infrastructure") != string::npos) {
        return "DevOps Engineer";
    }
    
    // Product roles
    if (lowerDesc.find("product manager") != string::npos ||
        lowerDesc.find("product owner") != string::npos) {
        return "Product Manager";
    }
    
    // Design roles
    if (lowerDesc.find("ui/ux") != string::npos ||
        lowerDesc.find("designer") != string::npos ||
        lowerDesc.find("user experience") != string::npos) {
        return "UI/UX Designer";
    }
    
    return "Other";
}

string extractJobSkills(const string& description) {
    size_t pos = description.find(" needed with experience in ");
    if (pos != string::npos) {
        string skills = description.substr(pos + 26); // 26 is length of " needed with experience in "
        size_t dotPos = skills.find('.');
        if (dotPos != string::npos) {
            skills = skills.substr(0, dotPos);
        }
        // Trim leading and trailing whitespace
        skills.erase(0, skills.find_first_not_of(" \t"));
        skills.erase(skills.find_last_not_of(" \t") + 1);
        return skills;
    }
    return description;
}

map<string, int> getAllSkillCounts() {
    static map<string, int> cachedCounts;
    static bool isLoaded = false;
    
    if (isLoaded) return cachedCounts;
    
    ifstream file("resume.csv");
    string line;
    
    if (!file.is_open()) return cachedCounts;
    
    getline(file, line);
    
    while (getline(file, line)) {
        if (!line.empty()) {
            if (line.front() == '"' && line.back() == '"') {
                line = line.substr(1, line.length() - 2);
            }
            
            size_t skillsPos = line.find("Experienced professional skilled in ");
            if (skillsPos != string::npos) {
                string skills = line.substr(skillsPos + 35);
                size_t endPos = skills.find(".");
                if (endPos != string::npos) {
                    skills = skills.substr(0, endPos);
                }
                
                stringstream ss(skills);
                string skill;
                while (getline(ss, skill, ',')) {
                    skill.erase(0, skill.find_first_not_of(" \t"));
                    skill.erase(skill.find_last_not_of(" \t") + 1);
                    
                    if (!skill.empty()) {
                        cachedCounts[skill]++;
                    }
                }
            }
        }
    }
    file.close();
    isLoaded = true;
    return cachedCounts;
}

set<string> getOutlierWords() {
    static set<string> cachedOutliers;
    static bool isLoaded = false;
    
    if (isLoaded) return cachedOutliers;
    
    map<string, int> skillCounts = getAllSkillCounts();
    
    for (const auto& pair : skillCounts) {
        if (pair.second < 1000) {
            cachedOutliers.insert(toLowerCase(pair.first));
        }
    }
    isLoaded = true;
    return cachedOutliers;
}

vector<string> filterSkills(const string& skillsText) {
    set<string> outliers = getOutlierWords();
    vector<string> validSkills;
    stringstream ss(skillsText);
    string skill;
    
    while (getline(ss, skill, ',')) {
        skill.erase(0, skill.find_first_not_of(" \t"));
        skill.erase(skill.find_last_not_of(" \t") + 1);
        
        string lowerSkill = toLowerCase(skill);
        if (!skill.empty() && outliers.find(lowerSkill) == outliers.end()) {
            validSkills.push_back(skill);
        }
    }
    return validSkills;
}

void showOutliers() {
    map<string, int> skillCounts = getAllSkillCounts();
    
    cout << "\n=== OUTLIER WORDS (COUNT < 1000) ===" << endl;
    int outlierCount = 0;
    
    for (const auto& pair : skillCounts) {
        if (pair.second < 1000) {
            cout << pair.first << ": " << pair.second << " times" << endl;
            outlierCount++;
        }
    }
    cout << "Total outlier types: " << outlierCount << endl;
    
    cout << "\n=== FREQUENT SKILLS (COUNT >= 1000) ===" << endl;
    int frequentCount = 0;
    
    for (const auto& pair : skillCounts) {
        if (pair.second >= 1000) {
            cout << pair.first << ": " << pair.second << " times" << endl;
            frequentCount++;
        }
    }
    cout << "Total frequent skills: " << frequentCount << endl;
}

void exportCleanedJobs() {
    ifstream file("job_description.csv");
    ofstream outFile("cleaned_job_descriptions.csv");
    
    if (!file.is_open() || !outFile.is_open()) {
        cout << "Error: Could not open files" << endl;
        return;
    }
    
    string line;
    int originalId = 1;
    
    outFile << "ID,Category,Skills" << endl;
    getline(file, line);
    
    while (getline(file, line)) {
        if (!line.empty()) {
            if (line.front() == '"' && line.back() == '"') {
                line = line.substr(1, line.length() - 2);
            }
            
            string skills = extractJobSkills(line);
            string category = classifyJob(line);
            outFile << originalId << "," << category << ",\"" << skills << "\"" << endl;
            originalId++;
        }
    }
    
    file.close();
    outFile.close();
    cout << "Cleaned job descriptions exported to cleaned_job_descriptions.csv" << endl;
}

void exportCleanedResumes() {
    ifstream file("resume.csv");
    ofstream outFile("cleaned_resumes.csv");
    
    if (!file.is_open() || !outFile.is_open()) {
        cout << "Error: Could not open files" << endl;
        return;
    }
    
    string line;
    int originalId = 1;
    
    outFile << "ID,Skills" << endl;
    getline(file, line);
    
    while (getline(file, line)) {
        if (!line.empty()) {
            if (line.front() == '"' && line.back() == '"') {
                line = line.substr(1, line.length() - 2);
            }
            
            size_t skillsPos = line.find("Experienced professional skilled in ");
            if (skillsPos != string::npos) {
                string skills = line.substr(skillsPos + 35);
                size_t endPos = skills.find(".");
                if (endPos != string::npos) {
                    skills = skills.substr(0, endPos);
                }
                
                vector<string> validSkills = filterSkills(skills);
                outFile << originalId << ",\"";
                for (size_t i = 0; i < validSkills.size(); i++) {
                    outFile << validSkills[i];
                    if (i < validSkills.size() - 1) outFile << ", ";
                }
                outFile << "\"" << endl;
            } else {
                outFile << originalId << ",\"No skills found\"" << endl;
            }
            originalId++;
        }
    }
    
    file.close();
    outFile.close();
    cout << "Cleaned resumes exported to cleaned_resumes.csv" << endl;
}

void loadJobDescriptions(int limit = -1)
{
    ifstream file("job_description.csv");
    if (!file.is_open())
    {
        cout << "Error: Could not open job_description.csv" << endl;
        return;
    }

    string line;
    int originalId = 1;
    int count = 0;
    vector<JobData> jobs;

    getline(file, line); // Skip header

    cout << "=== JOB DESCRIPTIONS WITH CLASSIFICATION ===" << endl;
    while (getline(file, line) && (limit == -1 || count < limit))
    {
        if (!line.empty())
        {
            // Remove quotes
            if (line.front() == '"' && line.back() == '"')
            {
                line = line.substr(1, line.length() - 2);
            }

            string skills = extractJobSkills(line);
            string category = classifyJob(line);
            jobs.push_back(JobData(originalId, skills, category));

            cout << "ID: " << originalId << endl;
            cout << "Category: " << category << endl;
            cout << "Skills: " << skills << endl;
            cout << "---" << endl;
            count++;
        }
        originalId++;
    }
    file.close();

    // Show classification summary
    cout << "\n=== CLASSIFICATION SUMMARY ===" << endl;
    vector<string> categories = {"Data Analyst", "ML Engineer", "Software Engineer", "DevOps Engineer", "Product Manager", "UI/UX Designer", "Other"};
    
    for (const string& cat : categories) {
        int catCount = 0;
        for (const JobData& job : jobs) {
            if (job.category == cat) catCount++;
        }
        if (catCount > 0) {
            cout << cat << ": " << catCount << " jobs" << endl;
        }
    }
}

void loadJobsByCategory(const string& targetCategory)
{
    ifstream file("job_description.csv");
    if (!file.is_open())
    {
        cout << "Error: Could not open job_description.csv" << endl;
        return;
    }

    string line;
    int originalId = 1;
    int count = 0;

    getline(file, line); // Skip header

    cout << "=== " << targetCategory << " JOBS ===" << endl;
    while (getline(file, line))
    {
        if (!line.empty())
        {
            // Remove quotes
            if (line.front() == '"' && line.back() == '"')
            {
                line = line.substr(1, line.length() - 2);
            }

            string category = classifyJob(line);
            if (category == targetCategory)
            {
                string skills = extractJobSkills(line);

                cout << "ID: " << originalId << endl;
                cout << "Skills: " << skills << endl;
                cout << "---" << endl;
                count++;
            }
            originalId++;
        }
    }
    file.close();
    
    if (count == 0) {
        cout << "No jobs found in this category." << endl;
    } else {
        cout << "\nTotal " << targetCategory << " jobs: " << count << endl;
    }
}

void loadResumes(int limit = -1, bool filterOutliers = true)
{
    ifstream file("resume.csv");
    if (!file.is_open())
    {
        cout << "Error: Could not open resume.csv" << endl;
        return;
    }

    string line;
    int originalId = 1;
    int count = 0;

    getline(file, line);

    cout << "\n=== RESUME SKILLS" << (filterOutliers ? " (FILTERED)" : " (UNFILTERED)") << " ===" << endl;
    while (getline(file, line) && (limit == -1 || count < limit))
    {
        if (!line.empty())
        {
            if (line.front() == '"' && line.back() == '"')
            {
                line = line.substr(1, line.length() - 2);
            }

            size_t skillsPos = line.find("Experienced professional skilled in ");
            if (skillsPos != string::npos)
            {
                string skills = line.substr(skillsPos + 35);
                
                size_t endPos = skills.find(".");
                if (endPos != string::npos)
                {
                    skills = skills.substr(0, endPos);
                }

                cout << "ID: " << originalId << endl;
                if (filterOutliers) {
                    vector<string> validSkills = filterSkills(skills);
                    for (size_t i = 0; i < validSkills.size(); i++) {
                        cout << validSkills[i];
                        if (i < validSkills.size() - 1) cout << ", ";
                    }
                    cout << endl;
                } else {
                    cout << skills << endl;
                }
                cout << "---" << endl;
            }
            else
            {
                cout << "ID: " << originalId << endl;
                cout << "No skills section found" << endl;
                cout << "---" << endl;
            }
            count++;
        }
        originalId++;
    }
    file.close();
}

void showCategoryMenu() {
    cout << "\n=== SELECT JOB CATEGORY ===" << endl;
    cout << "1. Data Analyst" << endl;
    cout << "2. ML Engineer" << endl;
    cout << "3. Software Engineer" << endl;
    cout << "4. DevOps Engineer" << endl;
    cout << "5. Product Manager" << endl;
    cout << "6. UI/UX Designer" << endl;
    cout << "7. Other" << endl;
    cout << "Enter your choice: ";
}

void showMenu() {
    cout << "\n=== JOB DESCRIPTION & RESUME ANALYZER ===" << endl;
    cout << "1. Display top 100 job descriptions" << endl;
    cout << "2. Display all job descriptions" << endl;
    cout << "3. Display jobs by category" << endl;
    cout << "4. Display top 50 resumes (FILTERED)" << endl;
    cout << "5. Display all resumes (FILTERED)" << endl;
    cout << "6. Display top 50 resumes (UNFILTERED)" << endl;
    cout << "7. Display all resumes (UNFILTERED)" << endl;
    cout << "8. Show outlier words with counts" << endl;
    cout << "9. Export cleaned job descriptions" << endl;
    cout << "10. Export cleaned resumes" << endl;
    cout << "11. Exit" << endl;
    cout << "Enter your choice: ";
}

int main()
{
    int choice, categoryChoice;
    vector<string> categories = {"Data Analyst", "ML Engineer", "Software Engineer", "DevOps Engineer", "Product Manager", "UI/UX Designer", "Other"};
    
    while (true) {
        showMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                loadJobDescriptions(100);
                break;
            case 2:
                loadJobDescriptions();
                break;
            case 3:
                showCategoryMenu();
                cin >> categoryChoice;
                if (categoryChoice >= 1 && categoryChoice <= 7) {
                    loadJobsByCategory(categories[categoryChoice - 1]);
                } else {
                    cout << "Invalid category choice." << endl;
                }
                break;
            case 4:
                loadResumes(50, true);
                break;
            case 5:
                loadResumes(-1, true);
                break;
            case 6:
                loadResumes(50, false);
                break;
            case 7:
                loadResumes(-1, false);
                break;
            case 8:
                showOutliers();
                break;
            case 9:
                exportCleanedJobs();
                break;
            case 10:
                exportCleanedResumes();
                break;
            case 11:
                cout << "Goodbye!" << endl;
                return 0;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
}
